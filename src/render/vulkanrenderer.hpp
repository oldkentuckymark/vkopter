#pragma once

#include <vulkan/vulkan.hpp>

#include <array>
#include <string>
#include <algorithm>

#include "util/fixed_vector.hpp"
#include "util/read_file.hpp"
#include "util/stb_image.h"

#include "vulkanwindow.hpp"

#include "memorymanager.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "light.hpp"
#include "renderobject.hpp"
#include "game/camera.hpp"
#include "game/citygen/grid.hpp"


#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/transform2.hpp"
#include "glm/ext/matrix_clip_space.hpp"


namespace vkopter::render
{

class VulkanRenderer
{
public:
    enum class PIPELINE_TYPE : uint32_t
    {
        MESH = 0,
        TERRAIN,
        WATER,
        NUM_PIPELINE_TYPES
    };
    template<class T> using per_frame_in_flight_vector = std::vector<T>;
    template<class T> using per_pipeline_type_array = std::array<T,static_cast<size_t>(PIPELINE_TYPE::NUM_PIPELINE_TYPES)>;


    VulkanRenderer(VulkanWindow& w, MemoryManager &mm) :
        window_(w),
        MAX_FRAMES_IN_FLIGHT(window_.concurrentFrameCount()),
        memory_manager_(mm)
    {
        initResources();
        initSwapChainResources();

        auto pdp = physical_device_.getProperties();
        if(pdp.limits.maxUniformBufferRange < 1000000000u)
        {
            twimtbp_ = true;
        }
    }

    ~VulkanRenderer()
    {
        releaseSwapChainResources();
        releaseResources();
    }

    VulkanRenderer(VulkanRenderer&) = delete;
    auto operator=(VulkanRenderer&) -> VulkanRenderer& = delete;
    VulkanRenderer(VulkanRenderer&&) = delete;
    auto operator=(VulkanRenderer&&) -> VulkanRenderer = delete;

    auto initResources() -> void
    {
        instance_ = window_.getInstance();
        device_ = window_.getDevice();
        physical_device_ = window_.getPhysicalDevice();
        command_pool_ = window_.getGraphicsCommandPool();
        queue_ = window_.getGraphicsQueue();


        clear_values_[0].setColor(clear_color_);
        clear_values_[1].setDepthStencil(clear_depth_stencil_);

        materials_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        model_matricies_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        lights_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        cameras_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        draw_commands_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

        render_objects_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        positions_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        texcoords_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        normals_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        indicies_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

        terrain_alts_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
        terrain_ters_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

        grid_buffers_.resize(MAX_FRAMES_IN_FLIGHT);

        init_buffers();
        init_descriptor_pool();

        descriptor_set_layout_ = create_descriptor_set_layout();
        pipeline_layout_ = create_pipeline_layout();

        pipelines_[static_cast<uint32_t>(PIPELINE_TYPE::MESH)] = create_pipeline("data/shaders/phong/vert.spv","data/shaders/phong/frag.spv","","",PIPELINE_TYPE::MESH);
        pipelines_[static_cast<uint32_t>(PIPELINE_TYPE::TERRAIN)] = create_pipeline("data/shaders/terrain/vert.spv","data/shaders/terrain/frag.spv","","",PIPELINE_TYPE::TERRAIN);
        pipelines_[static_cast<uint32_t>(PIPELINE_TYPE::WATER)] = create_pipeline("data/shaders/water/vert.spv","data/shaders/water/frag.spv","data/shaders/water/tesc.spv","data/shaders/water/tese.spv",PIPELINE_TYPE::WATER);



        //create texture atlas and image view
        int cif = 0;
        auto* data = stbi_load("data/textures/texture.png",&texture_atlas_width_,&texture_atlas_height_,&cif,4);

        texture_atlas_ = memory_manager_.createImage(texture_atlas_width_,texture_atlas_height_, vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,data);

        stbi_image_free(data);

        vk::ImageViewCreateInfo ivci = {};
        ivci.image = texture_atlas_;
        ivci.viewType = vk::ImageViewType::e2D;
        ivci.format = vk::Format::eR8G8B8A8Unorm;
        ivci.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.layerCount = 1;
        ivci.subresourceRange.levelCount = 1;
        texture_atlas_view_ = device_.createImageView(ivci);

        vk::SamplerCreateInfo sci = {};
        sci.minFilter = vk::Filter::eNearest;
        sci.magFilter = vk::Filter::eNearest;
        sci.anisotropyEnable = vk::False;
        sci.mipmapMode = vk::SamplerMipmapMode::eNearest;
        sci.unnormalizedCoordinates = vk::False;
        texture_atlas_sampler_ = device_.createSampler(sci);
        ///////////////



        descriptor_sets_.resize(MAX_FRAMES_IN_FLIGHT);
        descriptor_sets_ = allocate_descriptor_sets();
        update_descriptor_sets();


        //first 14 meshes MUST be terrain
        createMesh("data/meshes/00.gltf");
        createMesh("data/meshes/01.gltf");
        createMesh("data/meshes/02.gltf");
        createMesh("data/meshes/03.gltf");
        createMesh("data/meshes/04.gltf");
        createMesh("data/meshes/05.gltf");
        createMesh("data/meshes/06.gltf");
        createMesh("data/meshes/07.gltf");
        createMesh("data/meshes/08.gltf");
        createMesh("data/meshes/09.gltf");
        createMesh("data/meshes/10.gltf");
        createMesh("data/meshes/11.gltf");
        createMesh("data/meshes/12.gltf");
        createMesh("data/meshes/13.gltf");

        //15th mesh aka meshes_[14] must be the water
        createMesh("data/meshes/plane256.gltf");







    }

    auto initSwapChainResources() -> void
    {
        viewport_.setX(0.0f);
        viewport_.setY(0.0f);
        viewport_.setWidth(window_.swapChainImageSize().width);
        viewport_.setHeight(window_.swapChainImageSize().height);
        viewport_.setMinDepth(0.0f);
        viewport_.setMaxDepth(1.0f);
        scissor_.setOffset({0,0});
        scissor_.setExtent({static_cast<uint32_t>(static_cast<uint32_t>(window_.swapChainImageSize().width)),static_cast<uint32_t>(static_cast<uint32_t>(window_.swapChainImageSize().height))});
    }

    auto releaseSwapChainResources() -> void
    {

    }

    auto releaseResources() -> void
    {
        device_.waitIdle();
        device_.destroyDescriptorSetLayout(descriptor_set_layout_);
        device_.destroyDescriptorPool(descriptor_pool_);
        device_.destroyPipelineLayout(pipeline_layout_);
        for(auto& p : pipelines_) {device_.destroyPipeline(p);}

        auto destroyBuffers = [this](std::vector<vk::Buffer>& v) { for (auto& b : v) {memory_manager_.destroyBuffer(b);} };

        destroyBuffers(model_matricies_buffers_);
        destroyBuffers(materials_buffers_);
        destroyBuffers(render_objects_buffers_);
        destroyBuffers(lights_buffers_);
        destroyBuffers(cameras_buffers_);
        destroyBuffers(positions_buffers_);
        destroyBuffers(texcoords_buffers_);
        destroyBuffers(normals_buffers_);
        destroyBuffers(indicies_buffers_);
        destroyBuffers(draw_commands_buffers_);
        destroyBuffers(terrain_alts_buffers_);
        destroyBuffers(terrain_ters_buffers_);

        for(uint32_t m = 0; m <= 13; ++m)
        {
            removeMesh(m);
        }

        device_.destroyImageView(texture_atlas_view_);
        device_.destroySampler(texture_atlas_sampler_);

        memory_manager_.destroyImage(texture_atlas_);




    }






    auto startNextFrame() -> void
    {
        window_.beginFrame();

        auto currentFrame = window_.currentFrame();
        setClearColor(20/255.0f,20/255.0f,245/255.0f,1.0f);


        memory_manager_.updateBuffer(materials_buffers_[currentFrame], 0,
                                     materials_.getCurrentSizeInBytes(),
                                     materials_.data());


        memory_manager_.updateBuffer(model_matricies_buffers_[currentFrame], 0,
                                     model_matricies_.getCurrentSizeInBytes(),
                                     model_matricies_.data());




        memory_manager_.updateBuffer(cameras_buffers_[currentFrame], 0,
                                     cameras_.getCurrentSizeInBytes(),
                                     cameras_.data());


        memory_manager_.updateBuffer( lights_buffers_[currentFrame], 0,
                                      lights_.getCurrentSizeInBytes(),
                                      lights_.data());



        //loop through all render objects per frame, count mesh instances,upload vertbuffers, patch mesh infos
        //group render_objets for instacning, adject base index for shader lookup, upload ROs, use meshinfos for drawindirect commands

        std::vector<RenderObject> renobjs; renobjs.reserve(render_objects_.getSize());
        for(auto& o : render_objects_)
        {
            renobjs.emplace_back(o);
        }

        std::sort(renobjs.begin(),renobjs.end(),[](RenderObject l, RenderObject r){return l.mesh < r.mesh;});
        std::map<uint32_t, uint32_t> meshCounts;


        for(auto const & ro : render_objects_)
        {
            ++meshCounts[ro.mesh];
        }

        uint32_t firstInstance = 0;

        std::vector<vk::DrawIndexedIndirectCommand> diics;
        diics.reserve(256);

        std::vector<uint32_t> indicies;
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> texcoords;
        std::vector<glm::vec4> normals;


        //first 14 meshes must always be terrain
         for(uint32_t m = 0; m <= 13; ++m)
         {
             positions.insert(positions.end(), meshes_[m].positions().begin(),meshes_[m].positions().end());
             texcoords.insert(texcoords.end(), meshes_[m].texcoords().begin(),meshes_[m].texcoords().end());
             normals.insert(normals.end(), meshes_[m].normals().begin(),meshes_[m].normals().end());
             indicies.insert(indicies.end(), meshes_[m].indicies().begin(), meshes_[m].indicies().end());
         }

        //insert water mesh as 15th aka meshes_[14]
        positions.insert(positions.end(),meshes_[14].positions().begin(), meshes_[14].positions().end());
        texcoords.insert(texcoords.end(), meshes_[14].texcoords().begin(), meshes_[14].texcoords().end());
        normals.insert(normals.end(), meshes_[14].normals().begin(), meshes_[14].normals().end());
        indicies.insert(indicies.end(), meshes_[14].indicies().begin(), meshes_[14].indicies().end());

        for(auto & meshCount : meshCounts)
        {
            Mesh const & mesh = meshes_[meshCount.first];
            uint32_t const count = meshCount.second;

            positions.insert(positions.end(), mesh.positions().begin(), mesh.positions().end());
            texcoords.insert(texcoords.end(), mesh.texcoords().begin(), mesh.texcoords().end());
            normals.insert(normals.end(), mesh.normals().begin(), mesh.normals().end());
            indicies.insert(indicies.end(), mesh.indicies().begin(), mesh.indicies().end());

            vk::DrawIndexedIndirectCommand diic;
            diic.setInstanceCount(count);
            diic.setFirstInstance(firstInstance);
            diic.setFirstIndex(indicies.size() - mesh.indicies().size());
            diic.setIndexCount(mesh.indicies().size());
            diic.setVertexOffset(positions.size() - mesh.positions().size());
            diics.emplace_back(diic);

            firstInstance += count;
        }

        memory_manager_.updateBuffer(positions_buffers_[currentFrame], 0, positions.size()*sizeof(glm::vec4), positions.data());
        memory_manager_.updateBuffer(indicies_buffers_[currentFrame],0,indicies.size()*sizeof(uint32_t),indicies.data());
        memory_manager_.updateBuffer(texcoords_buffers_[currentFrame], 0, texcoords.size()*sizeof(glm::vec4), texcoords.data());
        memory_manager_.updateBuffer(normals_buffers_[currentFrame], 0, normals.size()*sizeof(glm::vec4), normals.data());


        memory_manager_.updateBuffer(render_objects_buffers_[currentFrame], 0, render_objects_.getSize()*sizeof(RenderObject), render_objects_.data());
        memory_manager_.updateBuffer(draw_commands_buffers_[currentFrame], 0, diics.size() * sizeof(vk::DrawIndexedIndirectCommand), diics.data());

        vk::RenderPassBeginInfo rpBeginInfo;
        rpBeginInfo.renderPass = window_.defaultRenderPass();
        rpBeginInfo.framebuffer = window_.currentFramebuffer();
        vk::Extent2D const sz = window_.swapChainImageSize();
        rpBeginInfo.setRenderArea( {{0,0}, {static_cast<uint32_t>(static_cast<uint32_t>(static_cast<uint32_t>(sz.width))),static_cast<uint32_t>(static_cast<uint32_t>(sz.height))}} );
        rpBeginInfo.setClearValues(clear_values_);
        vk::CommandBuffer cmdbuf = window_.currentCommandBuffer();


        cmdbuf.beginRenderPass( &rpBeginInfo, vk::SubpassContents::eInline );
        cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[static_cast<uint32_t>(PIPELINE_TYPE::MESH)]);
        cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,descriptor_sets_[currentFrame], nullptr);
        cmdbuf.pushConstants(pipeline_layout_,vk::ShaderStageFlagBits::eAll,0,sizeof(push_constant_struct_), &push_constant_struct_);
        cmdbuf.setViewport(0,viewport_);
        cmdbuf.setScissor(0,scissor_);

        cmdbuf.bindIndexBuffer(indicies_buffers_[currentFrame],0,vk::IndexType::eUint32);
        cmdbuf.bindVertexBuffers(0,positions_buffers_[currentFrame], {0});
        cmdbuf.bindVertexBuffers(1,texcoords_buffers_[currentFrame], {0});
        cmdbuf.bindVertexBuffers(2,normals_buffers_[currentFrame], {0});

        cmdbuf.drawIndexedIndirect(draw_commands_buffers_[currentFrame],0,diics.size(),sizeof(vk::DrawIndexedIndirectCommand));
        if(twimtbp_) {device_.waitIdle();}

        //terrain rendering
        cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[static_cast<uint32_t>(PIPELINE_TYPE::TERRAIN)]);
        cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,descriptor_sets_[currentFrame], nullptr);
        cmdbuf.pushConstants(pipeline_layout_,vk::ShaderStageFlagBits::eAll,0,sizeof(push_constant_struct_), &push_constant_struct_);
        cmdbuf.setViewport(0,viewport_);
        cmdbuf.setScissor(0,scissor_);

        cmdbuf.bindIndexBuffer(indicies_buffers_[currentFrame],0,vk::IndexType::eUint32);
        cmdbuf.bindVertexBuffers(0,positions_buffers_[currentFrame], {0});
        cmdbuf.bindVertexBuffers(1,texcoords_buffers_[currentFrame], {0});
        cmdbuf.bindVertexBuffers(2,normals_buffers_[currentFrame], {0});

        cmdbuf.drawIndexed(TERRAIN_MESH_INDEX_COUNT,terrain_width_*terrain_height_,0,0,0);


        if(twimtbp_) {device_.waitIdle();}

        //water rendering
        cmdbuf.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelines_[static_cast<uint32_t>(PIPELINE_TYPE::WATER)]);
        cmdbuf.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout_, 0,descriptor_sets_[currentFrame], nullptr);
        cmdbuf.pushConstants(pipeline_layout_,vk::ShaderStageFlagBits::eAll,0,sizeof(push_constant_struct_), &push_constant_struct_);
        cmdbuf.setViewport(0,viewport_);
        cmdbuf.setScissor(0,scissor_);

        cmdbuf.bindIndexBuffer(indicies_buffers_[currentFrame],0,vk::IndexType::eUint32);
        cmdbuf.bindVertexBuffers(0,positions_buffers_[currentFrame], {0});
        cmdbuf.bindVertexBuffers(1,texcoords_buffers_[currentFrame], {0});
        cmdbuf.bindVertexBuffers(2,normals_buffers_[currentFrame], {0});

        //draw water here!!!!!!
        //cmdbuf.draw(water_mesh_.getPositions().size(), 1, TERRAIN_MESH_VERT_COUNT * 14, 0);


        if(twimtbp_) {device_.waitIdle();}


        cmdbuf.endRenderPass();
        if(twimtbp_) {device_.waitIdle();}



        window_.endFrame();
    }




    auto setClearColor(float const r, float const g, float const b, float const a) -> void
    {
        clear_color_.setFloat32({r,g,b,a});
        clear_values_[0].setColor(clear_color_);
        clear_values_[1].setDepthStencil(clear_depth_stencil_);
    }

    auto createMesh(std::string const & path) -> uint32_t
    {
        uint32_t h = meshes_.emplace(path);
        mesh_handles_.push_back(h);
        return h;
    }

    auto removeMesh(uint32_t const i) -> void
    {
        meshes_.erase(i);
        mesh_handles_.erase( std::find(mesh_handles_.begin(),mesh_handles_.end(), i) );
    }

    auto createMaterial() -> uint32_t
    {
        return materials_.insert({});
    }

    auto getMaterialRef(uint32_t const i) -> Material&
    {
        return materials_[i];
    }

    auto removeMaterial(uint32_t const i) -> void
    {
        materials_.erase(i);
    }

    auto createModelMatrix() -> uint32_t
    {
        return model_matricies_.insert({});
    }

    auto getModelMatrixRef(uint32_t const i) -> glm::mat4x4&
    {
        return model_matricies_[i];
    }

    auto removeModelMatrix(uint32_t const i) -> void
    {
        model_matricies_.erase(i);
    }

    auto createLight() -> uint32_t
    {

        return lights_.insert({});
    }

    auto getLightRef(uint32_t const i) -> Light&
    {
        return lights_[i];
    }

    auto removeLight(uint32_t const i) -> void
    {
        lights_.erase(i);
    }

    auto createCamera() -> uint32_t
    {
        return cameras_.insert({});
    }

    auto getCameraRef(uint32_t const i) -> game::Camera&
    {
        return cameras_[i];
    }

    auto removeCamera(uint32_t const i) -> void
    {
        cameras_.erase(i);
    }

    auto createRenderObject(vkopter::render::RenderObject const ro) -> uint32_t
    {
        return render_objects_.emplace(ro);
    }

    auto geRenderObjectRef(uint32_t const i) -> RenderObject&
    {
        return render_objects_[i];
    }

    auto removeRenderObject(uint32_t const i) -> void
    {
        render_objects_.erase(i);
    }

    auto updateTerrain(uint32_t * ters, uint32_t * alts) -> void
    {
        for(auto& b : terrain_alts_buffers_)
        {
            memory_manager_.updateBuffer(b,0,terrain_width_*terrain_height_*sizeof(uint32_t), alts);
        }
        for(auto& b : terrain_ters_buffers_)
        {
            memory_manager_.updateBuffer(b,0,terrain_width_*terrain_height_*sizeof(uint32_t), ters);
        }
    }

    auto resizeTerrain(uint32_t const w, uint32_t const h) -> void
    {
        terrain_width_ = w;
        terrain_height_ = h;
    }

    template<uint64_t W, uint64_t H, uint64_t D>
    auto updateGrid(std::array<game::citygen::Atom, W*H*D>& atoms) -> void
    {
        for(auto& b : grid_buffers_)
        {
            memory_manager_.updateBuffer(b, 0, sizeof(game::citygen::Atom)*W*H*D, atoms.data());
        }
    }

private:

    auto init_buffers() -> void
    {
        for(auto i = 0ul; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            materials_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,materials_.getMaxSizeInBytes());
            model_matricies_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,model_matricies_.getMaxSizeInBytes());
            lights_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,lights_.getMaxSizeInBytes());
            cameras_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,cameras_.getMaxSizeInBytes());
            render_objects_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,sizeof(RenderObject) * MAX_OBJECTS_COUNT);
            positions_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,sizeof(glm::vec4) * MAX_VERTEX_COUNT);
            texcoords_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,sizeof(glm::vec4) * MAX_VERTEX_COUNT);
            normals_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,sizeof(glm::vec4) * MAX_VERTEX_COUNT);

            terrain_alts_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,sizeof(uint32_t) * MAX_TERRAIN_HEIGHT_* MAX_TERRAIN_WIDTH_);
            terrain_ters_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,sizeof(uint32_t) * MAX_TERRAIN_HEIGHT_* MAX_TERRAIN_WIDTH_);

            indicies_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndexBuffer,sizeof(uint32_t) * MAX_VERTEX_COUNT);
            draw_commands_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eIndirectBuffer,sizeof(vk::DrawIndexedIndirectCommand) * MAX_OBJECTS_COUNT);

            grid_buffers_[i] = memory_manager_.createBuffer(vk::BufferUsageFlagBits::eStorageBuffer, MAX_TERRAIN_WIDTH_ * MAX_TERRAIN_HEIGHT_*sizeof(game::citygen::Atom));

        }


    }

    auto init_descriptor_pool() -> void
    {
        vk::DescriptorPoolSize dps;
        dps.setType(vk::DescriptorType::eStorageBuffer);
        dps.setDescriptorCount(32 * MAX_FRAMES_IN_FLIGHT);

        vk::DescriptorPoolSize tps;
        tps.setType(vk::DescriptorType::eCombinedImageSampler);
        tps.setDescriptorCount(8 * MAX_FRAMES_IN_FLIGHT);

        std::array t{dps,tps};

        vk::DescriptorPoolCreateInfo dpci;
        dpci.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
        dpci.setMaxSets(4);
        dpci.setPoolSizes(t);
        descriptor_pool_ = device_.createDescriptorPool(dpci);
    }

    auto create_descriptor_set_layout() -> vk::DescriptorSetLayout
    {
        std::array<vk::DescriptorSetLayoutBinding,32> dslb;
        for(auto i = 0ul; i < 32; ++i)
        {
            dslb[i].setBinding(i);
            dslb[i].setDescriptorCount(1);
            dslb[i].setDescriptorType(vk::DescriptorType::eStorageBuffer);
            dslb[i].setStageFlags(vk::ShaderStageFlagBits::eAll);
        }
        dslb[12].binding = 12;
        dslb[12].descriptorCount = 1;
        dslb[12].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        dslb[12].pImmutableSamplers = nullptr;



        std::array<vk::DescriptorBindingFlags, 32> bf =
        {
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0),
            vk::DescriptorBindingFlags(0)
        };

        vk::DescriptorSetLayoutBindingFlagsCreateInfo dslbfci;
        dslbfci.setBindingFlags(bf);

        vk::DescriptorSetLayoutCreateInfo dslci;
        dslci.setPNext(&dslbfci);
        dslci.setBindings(dslb);
        dslci.setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool);

        vk::DescriptorSetLayout dsl = device_.createDescriptorSetLayout(dslci);
        return dsl;
    }

    auto allocate_descriptor_sets() -> per_frame_in_flight_vector<vk::DescriptorSet>
    {
        vk::DescriptorSetAllocateInfo dsai;
        dsai.setDescriptorPool(descriptor_pool_);
        std::vector<vk::DescriptorSetLayout> dsls;
        for(auto i = 0ul; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            dsls.push_back(descriptor_set_layout_);
        }
        dsai.setSetLayouts(dsls);
        return device_.allocateDescriptorSets(dsai);
    }

    auto create_pipeline_layout() -> vk::PipelineLayout
    {
        vk::PushConstantRange pcr;
        pcr.setSize(128);
        pcr.setOffset(0);
        pcr.setStageFlags(vk::ShaderStageFlagBits::eAll);

        vk::PipelineLayoutCreateInfo plci;
        plci.setPushConstantRanges(pcr);
        plci.setSetLayouts(descriptor_set_layout_);
        pipeline_layout_ = device_.createPipelineLayout(plci);
        return pipeline_layout_;
    }

    auto create_pipeline(std::string const & vertPath,
                        std::string const & fragPath,
                        std::string const & tcsPath,
                        std::string const & tesPath,
                        PIPELINE_TYPE renderType) -> vk::Pipeline
    {
        vk::ShaderModule vertModule = load_shader(vertPath);
        vk::ShaderModule fragModule = load_shader(fragPath);
        vk::ShaderModule tcsModule;
        vk::ShaderModule tesModule;

        if(renderType == PIPELINE_TYPE::WATER)
        {
            tcsModule = load_shader(tcsPath);
            tesModule = load_shader(tesPath);
        }

        vk::PipelineShaderStageCreateInfo tcsPSSCI;
        tcsPSSCI.setStage(vk::ShaderStageFlagBits::eTessellationControl);
        tcsPSSCI.setModule(tcsModule);
        tcsPSSCI.setPName("main");

        vk::PipelineShaderStageCreateInfo tesPSSCI;
        tesPSSCI.setStage(vk::ShaderStageFlagBits::eTessellationEvaluation);
        tesPSSCI.setModule(tesModule);
        tesPSSCI.setPName("main");

        vk::PipelineShaderStageCreateInfo vertPSSCI;
        vertPSSCI.setStage(vk::ShaderStageFlagBits::eVertex);
        vertPSSCI.setModule(vertModule);
        vertPSSCI.setPName("main");

        vk::PipelineShaderStageCreateInfo fragPSSCI;
        fragPSSCI.setStage(vk::ShaderStageFlagBits::eFragment);
        fragPSSCI.setModule(fragModule);
        fragPSSCI.setPName("main");

        std::array<vk::PipelineShaderStageCreateInfo,4> shaderStages = {vertPSSCI, fragPSSCI, tcsPSSCI, tesPSSCI};


        vk::PipelineVertexInputStateCreateInfo pvisci {};
        pvisci.setVertexAttributeDescriptionCount(0);
        pvisci.setVertexBindingDescriptionCount(0);

        vk::PipelineInputAssemblyStateCreateInfo piasci;
        piasci.setTopology(vk::PrimitiveTopology::eTriangleList);
        if(renderType == PIPELINE_TYPE::WATER)
        {
            piasci.setTopology(vk::PrimitiveTopology::ePatchList);
        }
        piasci.setPrimitiveRestartEnable(VK_FALSE);

        vk::PipelineDynamicStateCreateInfo pdsci;
        std::array<vk::DynamicState,4> dsa =
        {
            vk::DynamicState::eScissor,
            vk::DynamicState::eViewport,
            vk::DynamicState::eDepthBias,
            vk::DynamicState::eBlendConstants
        };
        pdsci.setDynamicStates(dsa);

        vk::Viewport vp;
        vp.setX(0.0f);
        vp.setY(0.0f);
        vp.setWidth(1.0f);
        vp.setHeight(1.0f);
        vk::Rect2D sc;
        sc.setOffset({0,0});
        sc.setExtent({1,1});

        vk::PipelineViewportStateCreateInfo pvsci;
        pvsci.setViewports(vp);
        pvsci.setScissors(sc);


        vk::PipelineRasterizationStateCreateInfo prsci;
        prsci.setDepthClampEnable(VK_FALSE);
        prsci.setRasterizerDiscardEnable(VK_FALSE);
        prsci.setPolygonMode(vk::PolygonMode::eFill);
        prsci.setLineWidth(1.0f);
        prsci.setCullMode(vk::CullModeFlagBits::eBack);
        prsci.setFrontFace(vk::FrontFace::eCounterClockwise);
        prsci.setDepthBiasEnable(VK_FALSE);
        if(renderType == PIPELINE_TYPE::WATER)
        {
            prsci.setCullMode(vk::CullModeFlagBits::eNone);
            prsci.setPolygonMode(vk::PolygonMode::eLine);
        }


        vk::PipelineMultisampleStateCreateInfo pmsci;
        pmsci.setSampleShadingEnable(VK_FALSE);
        pmsci.setRasterizationSamples(vk::SampleCountFlagBits::e1);

        vk::PipelineDepthStencilStateCreateInfo pdssci;
        pdssci.setDepthTestEnable(VK_TRUE);
        pdssci.setDepthWriteEnable(VK_TRUE);
        pdssci.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
        pdssci.setDepthBoundsTestEnable(VK_FALSE);
        pdssci.setStencilTestEnable(VK_FALSE);
        if(renderType == PIPELINE_TYPE::WATER)
        {

        }

        vk::PipelineColorBlendAttachmentState cbas;
        cbas.setColorWriteMask(vk::ColorComponentFlagBits::eR |
                               vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB |
                               vk::ColorComponentFlagBits::eA);

        cbas.setBlendEnable(VK_FALSE);
        if(renderType == PIPELINE_TYPE::WATER)
        {
            cbas.setBlendEnable(VK_TRUE);
            cbas.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            cbas.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            cbas.colorBlendOp = vk::BlendOp::eAdd;
            cbas.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            cbas.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            cbas.alphaBlendOp = vk::BlendOp::eAdd;
        }



        vk::PipelineColorBlendStateCreateInfo pcbsci;
        pcbsci.setLogicOpEnable(VK_FALSE);
        pcbsci.setAttachmentCount(1);
        pcbsci.setAttachments(cbas);


        vk::PipelineTessellationStateCreateInfo ptsci;
        ptsci.setPatchControlPoints(4);





        vk::GraphicsPipelineCreateInfo gpci;
        gpci.setPStages(shaderStages.data());
        int stageCount = 2;
        if(renderType == PIPELINE_TYPE::WATER)
        {
            stageCount = 4;
            gpci.setPTessellationState(&ptsci);
        }
        gpci.setStageCount(stageCount);
        gpci.setPVertexInputState(&pvisci);
        gpci.setPInputAssemblyState(&piasci);
        gpci.setPRasterizationState(&prsci);
        gpci.setPMultisampleState(&pmsci);
        gpci.setPDepthStencilState(&pdssci);
        gpci.setPColorBlendState(&pcbsci);
        gpci.setLayout(pipeline_layout_);
        gpci.setRenderPass(window_.defaultRenderPass());
        gpci.setSubpass(0);
        gpci.setPDynamicState(&pdsci);
        gpci.setPViewportState(&pvsci);
        vk::Pipeline pipeline = device_.createGraphicsPipeline(VK_NULL_HANDLE, gpci).value;


        device_.destroyShaderModule(vertModule);
        device_.destroyShaderModule(fragModule);
        if(renderType == PIPELINE_TYPE::WATER)
        {
            device_.destroyShaderModule(tcsModule);
            device_.destroyShaderModule(tesModule);
        }

        return pipeline;
    }

    auto update_descriptor_sets() -> void
    {
        std::vector<per_frame_in_flight_vector<vk::Buffer>> buffers(32);
        for(auto& b : buffers)
        {
            b.resize(MAX_FRAMES_IN_FLIGHT);
            for(auto& bb : b)
            {
                bb = VK_NULL_HANDLE;
            }
        }

        buffers[0] = indicies_buffers_;
        buffers[1] = positions_buffers_;
        buffers[2] = texcoords_buffers_;
        buffers[3] = normals_buffers_;
        buffers[4] = render_objects_buffers_;
        buffers[5] = materials_buffers_;
        buffers[6] = cameras_buffers_;
        buffers[7] = model_matricies_buffers_;
        buffers[8] = lights_buffers_;
        buffers[9] = terrain_ters_buffers_;
        buffers[10] = terrain_alts_buffers_;
        buffers[11] = grid_buffers_;


        for(auto f = 0ul; f < MAX_FRAMES_IN_FLIGHT; ++f)
        {
            std::vector<vk::WriteDescriptorSet> wds(13);
            std::vector<vk::DescriptorBufferInfo> dbis(wds.size());
            std::vector<vk::DescriptorImageInfo> diis(1);
            for(auto bindingNum = 0ul; bindingNum < wds.size(); ++bindingNum)
            {
                auto& wd = wds[bindingNum];
                auto& dbi = dbis[bindingNum];
                dbi.setBuffer(buffers[bindingNum][f]);
                dbi.setOffset(0);
                dbi.setRange(VK_WHOLE_SIZE);

                wd.setDstSet(descriptor_sets_[f]);
                wd.setDstBinding(bindingNum);
                wd.setDstArrayElement(0);
                wd.setDescriptorType(vk::DescriptorType::eStorageBuffer);
                wd.setDescriptorCount(1);
                wd.setBufferInfo(dbi);

            }
            for(auto bindingNum = 12ul; bindingNum < 13ul; ++bindingNum)
            {
                auto& wd = wds[bindingNum];
                auto& dii = diis[0];
                dii.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
                dii.setImageView(texture_atlas_view_);
                dii.setSampler(texture_atlas_sampler_);

                wd.setDstSet(descriptor_sets_[f]);
                wd.setDstBinding(bindingNum);
                wd.setDstArrayElement(0);
                wd.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
                wd.setDescriptorCount(1);
                wd.setImageInfo(dii);


            }



            device_.updateDescriptorSets(wds,{});
        }
    }

    auto load_shader(const std::string &path) -> vk::ShaderModule
    {
        auto buffer = read_file(path);

        vk::ShaderModuleCreateInfo smci;
        smci.setPCode(std::bit_cast<uint32_t*>(buffer.data()));
        smci.setCodeSize(buffer.size());

        return device_.createShaderModule(smci);
    }


    VulkanWindow& window_;

    constexpr size_t static MAX_OBJECTS_COUNT = 32768;
    constexpr size_t static MAX_VERTEX_COUNT = 1048576;
    uint32_t const MAX_FRAMES_IN_FLIGHT = 0;


    vk::Instance instance_;
    vk::Device device_;
    vk::PhysicalDevice physical_device_;
    vk::Queue queue_;
    vk::CommandPool command_pool_;

    MemoryManager& memory_manager_;


    vk::ClearColorValue clear_color_ = std::array<float,4>{1.0f, 0.0f, 0.0f,1.0f};
    vk::ClearDepthStencilValue clear_depth_stencil_ = {1, 0};

    std::array<vk::ClearValue,2> clear_values_ =
    {
        vk::ClearValue(vk::ClearColorValue(std::array<float,4>{0.0f,0.0f,0.0f,0.0f})),
        vk::ClearValue(vk::ClearDepthStencilValue(1.0f,0.0f))
    };
    vk::Viewport viewport_;
    vk::Rect2D scissor_;



    per_pipeline_type_array<vk::Pipeline> pipelines_;
    vk::PipelineLayout pipeline_layout_;

    vk::DescriptorPool descriptor_pool_;
    vk::DescriptorSetLayout descriptor_set_layout_;
    per_frame_in_flight_vector<vk::DescriptorSet> descriptor_sets_;


    FixedVector<Mesh,1024> meshes_;
    std::vector<uint32_t> mesh_handles_;

    vk::Image texture_atlas_;
    vk::ImageView texture_atlas_view_;
    vk::Sampler texture_atlas_sampler_;

    FixedVector<RenderObject,MAX_OBJECTS_COUNT> render_objects_;
    FixedVector<Material, MAX_OBJECTS_COUNT> materials_;
    FixedVector<game::Camera, 4> cameras_;
    FixedVector<glm::mat4, MAX_OBJECTS_COUNT> model_matricies_;
    FixedVector<Light, MAX_OBJECTS_COUNT> lights_;


    //gpu side data must be duplicated for each frame in flight

    per_frame_in_flight_vector<vk::Buffer> indicies_buffers_;
    per_frame_in_flight_vector<vk::Buffer> positions_buffers_;
    per_frame_in_flight_vector<vk::Buffer> texcoords_buffers_;
    per_frame_in_flight_vector<vk::Buffer> normals_buffers_;

    per_frame_in_flight_vector<vk::Buffer> render_objects_buffers_;
    per_frame_in_flight_vector<vk::Buffer> materials_buffers_;
    per_frame_in_flight_vector<vk::Buffer> cameras_buffers_;
    per_frame_in_flight_vector<vk::Buffer> model_matricies_buffers_;
    per_frame_in_flight_vector<vk::Buffer> lights_buffers_;


    per_frame_in_flight_vector<vk::Buffer> terrain_alts_buffers_;
    per_frame_in_flight_vector<vk::Buffer> terrain_ters_buffers_;

    per_frame_in_flight_vector<vk::Buffer> draw_commands_buffers_;

    per_frame_in_flight_vector<vk::Buffer> grid_buffers_;

    //push constants
    struct PushConstantStruct
    {
        int32_t numLights;
        float maxLightDistance;
        float oldTime;
        float currentTime;
        uint32_t terrainWidth;
        uint32_t terriaiHeight;
        uint32_t reseved6;
        uint32_t reseved7;
        uint32_t reseved8;
        uint32_t reseved9;
        uint32_t reseved10;
        uint32_t reseved11;
        uint32_t reseved12;
        uint32_t reseved13;
        uint32_t reseved14;
        uint32_t reseved15;
        uint32_t reseved16;
        uint32_t reseved17;
        uint32_t reseved18;
        uint32_t reseved19;
        uint32_t reseved20;
        uint32_t reseved21;
        uint32_t reseved22;
        uint32_t reseved23;
        uint32_t reseved24;
        uint32_t reseved25;
        uint32_t reseved26;
        uint32_t reseved27;
        uint32_t reseved28;
        uint32_t reseved29;
        uint32_t reseved30;
        uint32_t reseved31;
    };
    PushConstantStruct push_constant_struct_{};


    int texture_atlas_width_ = 0;
    int texture_atlas_height_ = 0;

    uint32_t terrain_width_ = 0;
    uint32_t terrain_height_ = 0;

    uint32_t const MAX_TERRAIN_WIDTH_ = 256;
    uint32_t const MAX_TERRAIN_HEIGHT_ = 256;
    uint32_t const TERRAIN_MESH_INDEX_COUNT = 36;
    uint32_t const TERRAIN_MESH_VERT_COUNT = 36;







    bool twimtbp_ = false;


};

}
