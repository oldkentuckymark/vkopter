#pragma once

#include "render/memorymanager.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <chrono>
#include <vulkan/vulkan.hpp>

#ifndef NDEBUG
#include <iostream>
#endif

#include <array>
#include <vector>

namespace vkopter {

class VulkanWindow
{
public:
    VulkanWindow(const int w, const int h)
        : window_(SDL_CreateWindow("vkopter",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   w,
                                   h,
                                   SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE))

    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);

        create_instance();
        pick_physical_device();
        get_queue_families();
        create_logical_device();
        memoryManager = new render::MemoryManager(instance_,
                                                  device_,
                                                  physical_device_,
                                                  graphicsQueueFamilyIndex());
        create_surface();

        create_swapchain();
        create_imageviews();
        create_depth_resourses();

        create_graphics_command_pool_and_buffer();
        create_default_renderpass();
        create_framebuffers();
        create_sync_objects();
    }

    ~VulkanWindow()
    {
        device_.waitIdle();
        destroy_sync_objects();
        destroy_framebuffers();
        destroy_default_renderpass();
        destroy_graphics_command_pool();
        destroy_depth_resourses();
        delete memoryManager;
        destroy_imageviews();
        destroy_swapchain();

        destroy_surface();
        destroy_logical_device();
        destroy_instance();
    }

    VulkanWindow(VulkanWindow const &) = delete;
    auto operator=(VulkanWindow const &) -> VulkanWindow & = delete;
    VulkanWindow(VulkanWindow &&) = delete;
    auto operator=(VulkanWindow &&) -> VulkanWindow & = delete;

    auto concurrentFrameCount() -> uint32_t { return concurrent_frame_count_; }

    auto currentFrame() -> uint32_t { return current_frame_; }

    auto getInstance() -> vk::Instance { return instance_; }

    auto getDevice() -> vk::Device { return device_; }

    auto getPhysicalDevice() -> vk::PhysicalDevice { return physical_device_; }

    auto getGraphicsCommandPool() -> vk::CommandPool { return default_command_pool_; }

    auto getGraphicsQueue() -> vk::Queue { return graphics_queue_; }

    auto graphicsQueueFamilyIndex() -> uint32_t { return graphics_queue_index_; }

    vk::Queue getTransferQueue() { return transfer_queue_; }

    uint32_t transferQueueFamilyIndex() { return transfer_queue_index_; }

    vk::Extent2D swapChainImageSize() { return swapchain_extent_; }

    vk::RenderPass defaultRenderPass() { return default_renderpass_; }

    vk::Framebuffer currentFramebuffer() { return swapchain_framebuffers_[current_frame_]; }

    vk::CommandBuffer currentCommandBuffer() { return default_command_buffers_[current_frame_]; }

    uint32_t getWidth() const
    {
        int w;
        SDL_GL_GetDrawableSize(window_, &w, nullptr);
        return w;
    }

    uint32_t getHeight() const
    {
        int h;
        SDL_GL_GetDrawableSize(window_, nullptr, &h);
        return h;
    }

    render::MemoryManager &getMemoryManager() { return *memoryManager; }

    void beginFrame()
    {
        auto r = device_.waitForFences(in_flight_fence, VK_TRUE, UINT64_MAX);
        device_.resetFences(in_flight_fence);

        current_frame_ = device_
                             .acquireNextImageKHR(swapchain_,
                                                  UINT64_MAX,
                                                  image_available_semaphore,
                                                  VK_NULL_HANDLE)
                             .value;

        default_command_buffers_[current_frame_].reset();

        vk::CommandBufferBeginInfo cbbi;
        default_command_buffers_[current_frame_].begin(cbbi);
    }

    void endFrame()
    {
        default_command_buffers_[current_frame_].end();
        vk::SubmitInfo submitInfo;
        vk::Semaphore waitSemaphores[] = {image_available_semaphore};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        submitInfo.setWaitSemaphoreCount(1);
        submitInfo.setWaitSemaphores(waitSemaphores);
        submitInfo.setWaitDstStageMask(waitStages);
        submitInfo.setCommandBufferCount(1);
        submitInfo.setCommandBuffers(default_command_buffers_[current_frame_]);

        vk::Semaphore signalSemaphores[] = {render_finished_semaphore};
        submitInfo.setSignalSemaphoreCount(1);
        submitInfo.setSignalSemaphores(signalSemaphores);

        graphics_queue_.submit(submitInfo, in_flight_fence);

        vk::PresentInfoKHR presentInfo;
        presentInfo.setWaitSemaphoreCount(1);
        presentInfo.setWaitSemaphores(signalSemaphores);
        presentInfo.setSwapchains(swapchain_);
        presentInfo.setImageIndices(current_frame_);

        auto r = graphics_queue_.presentKHR(presentInfo);
    }

    void recreate_swapchain()
    {
        device_.waitIdle();

        destroy_depth_resourses();
        destroy_imageviews();
        destroy_framebuffers();
        destroy_default_renderpass();
        destroy_swapchain();

        create_swapchain();
        create_imageviews();
        create_depth_resourses();
        create_default_renderpass();
        create_framebuffers();
    }

    void present() {}

private:
    std::chrono::time_point<std::chrono::steady_clock> c1, c2;

    SDL_Window *window_ = nullptr;

    uint32_t concurrent_frame_count_ = 0;
    uint32_t current_frame_ = 0;

    vk::Instance instance_;
    vk::PhysicalDevice physical_device_;
    vk::Device device_;

    uint32_t graphics_queue_index_ = UINT32_MAX;
    uint32_t transfer_queue_index_ = UINT32_MAX;
    vk::Queue graphics_queue_;
    vk::Queue transfer_queue_;

    vk::SurfaceKHR surface_;
    vk::SurfaceFormatKHR surface_format_;
    vk::ColorSpaceKHR surface_colorspace_;
    vk::PresentModeKHR present_mode_;
    vk::Extent2D swapchain_extent_;
    vk::SwapchainKHR swapchain_;
    std::vector<vk::Image> swapchain_images_;
    std::vector<vk::ImageView> swapchain_image_views_;
    std::vector<vk::Framebuffer> swapchain_framebuffers_;
    vk::Format depth_format_ = vk::Format::eD32Sfloat;
    vk::Image depth_image_;
    vk::ImageView depth_image_view_;

    vk::CommandPool default_command_pool_;
    std::vector<vk::CommandBuffer> default_command_buffers_;

    vk::RenderPass default_renderpass_;

    vk::Semaphore image_available_semaphore;
    vk::Semaphore render_finished_semaphore;
    vk::Fence in_flight_fence;

    render::MemoryManager *memoryManager = nullptr;

#ifndef NDEBUG
    VkDebugUtilsMessengerEXT debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData)
    {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
            std::abort();
        }

        return VK_FALSE;
    }
#endif

    void create_instance()
    {
        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "vkopter";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "vkopter";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        std::vector<char const *> enabledLayers;
#ifndef NDEBUG
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

        std::vector<char const *> enabledExtentions;
        unsigned int count;
        SDL_Vulkan_GetInstanceExtensions(window_, &count, nullptr);
        enabledExtentions.resize(count);
        SDL_Vulkan_GetInstanceExtensions(window_, &count, &enabledExtentions[0]);
#ifndef NDEBUG
        enabledExtentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = enabledExtentions.size();
        createInfo.ppEnabledExtensionNames = &enabledExtentions[0];
        createInfo.enabledLayerCount = enabledLayers.size();
        createInfo.ppEnabledLayerNames = &enabledLayers[0];
        instance_ = vk::createInstance(createInfo);

#ifndef NDEBUG
        VkDebugUtilsMessengerCreateInfoEXT dci{};
        dci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                              | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                          | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                          | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dci.pfnUserCallback = debugCallback;
        dci.pUserData = nullptr; // Optional
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
        func(instance_, &dci, nullptr, &debugMessenger);
#endif
    }

    void destroy_instance()
    {
#ifndef NDEBUG
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        func(instance_, debugMessenger, nullptr);
#endif
        instance_.destroy();
    }

    void pick_physical_device()
    {
        auto pdevs = instance_.enumeratePhysicalDevices();
        auto pp = pdevs[0].getProperties();
        physical_device_ = pdevs[0];
    }

    void get_queue_families()
    {
        auto dqfps = physical_device_.getQueueFamilyProperties();

        for (auto i = 0ul; i < dqfps.size(); ++i) {
            if (dqfps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                graphics_queue_index_ = i;
            } else if (dqfps[i].queueFlags & vk::QueueFlagBits::eTransfer) {
                transfer_queue_index_ = i;
            }
        }
        if (transfer_queue_index_ == UINT32_MAX) {
            transfer_queue_index_ = graphics_queue_index_;
        }
    }

    void create_logical_device()
    {
        std::vector<char const *> extentions;
        extentions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        std::array<float, 1> qp = {1.0f};

        std::array<vk::DeviceQueueCreateInfo, 2> dqci;
        dqci[0].setQueueCount(1);
        dqci[0].setQueueFamilyIndex(graphics_queue_index_);
        dqci[0].setQueuePriorities(qp);
        dqci[1].setQueueCount(1);
        dqci[1].setQueueFamilyIndex(transfer_queue_index_);
        dqci[1].setQueuePriorities(qp);

        vk::DeviceCreateInfo dci;
        vk::PhysicalDeviceDescriptorIndexingFeatures pddif;
        //pddif.setDescriptorBindingPartiallyBound(VK_TRUE);
        //pddif.setDescriptorBindingSampledImageUpdateAfterBind(VK_TRUE);
        //pddif.setDescriptorBindingVariableDescriptorCount(VK_TRUE);
        //pddif.setShaderSampledImageArrayNonUniformIndexing(VK_TRUE);
        //pddif.setShaderStorageBufferArrayNonUniformIndexing(VK_TRUE);
        dci.setPNext(&pddif);
        dci.setPQueueCreateInfos(dqci.data());
        dci.setQueueCreateInfoCount(2);
        if (graphics_queue_index_ == transfer_queue_index_) {
            dci.setQueueCreateInfoCount(1);
        }
        dci.setEnabledExtensionCount(extentions.size());
        dci.setPEnabledExtensionNames(extentions);

        vk::PhysicalDeviceFeatures pdf;
        pdf.setTessellationShader(VK_TRUE);
        pdf.setShaderUniformBufferArrayDynamicIndexing(VK_TRUE);
        pdf.setMultiDrawIndirect(VK_TRUE);
        pdf.setDrawIndirectFirstInstance(VK_TRUE);
        pdf.setFillModeNonSolid(VK_TRUE);
        dci.setPEnabledFeatures(&pdf);

        device_ = physical_device_.createDevice(dci);

        graphics_queue_ = device_.getQueue(graphics_queue_index_, 0);
        transfer_queue_ = device_.getQueue(transfer_queue_index_, 0);
    }

    void destroy_logical_device() { device_.destroy(); }

    void create_surface()
    {
        VkSurfaceKHR cs = surface_;
        bool const b = SDL_Vulkan_CreateSurface(window_, instance_, &cs);
        assert(b && "ERROR: could not create vulkan surface");
        surface_ = cs;
    }

    void destroy_surface() { instance_.destroySurfaceKHR(surface_); }

    void create_depth_resourses()
    {
        depth_image_ = memoryManager->createImage(swapchain_extent_.width,
                                                  swapchain_extent_.height,
                                                  depth_format_,
                                                  vk::ImageUsageFlagBits::eDepthStencilAttachment
                                                      | vk::ImageUsageFlagBits::eSampled);

        vk::ImageViewCreateInfo createInfo;
        createInfo.image = depth_image_;
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = depth_format_;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;
        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        depth_image_view_ = device_.createImageView(createInfo);
    }

    void destroy_depth_resourses()
    {
        device_.destroyImageView(depth_image_view_);
        memoryManager->destroyImage(depth_image_);
    }

    void create_swapchain()
    {
        auto surfCap = physical_device_.getSurfaceCapabilitiesKHR(surface_);
        auto surfaceFormats = physical_device_.getSurfaceFormatsKHR(surface_);
        auto presentModes = physical_device_.getSurfacePresentModesKHR(surface_);

        bool foundwanted = false;
        for (auto &sf : surfaceFormats) {
            if (sf.format == vk::Format::eB8G8R8A8Srgb
                && sf.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                surface_format_ = sf;
                foundwanted = true;
            }
        }
        if (!foundwanted) {
            surface_format_ = surfaceFormats[0];
        }

        present_mode_ = vk::PresentModeKHR::eFifo;

        swapchain_extent_.width = std::min(surfCap.maxImageExtent.width,
                                           std::max(getWidth(), surfCap.minImageExtent.width));
        swapchain_extent_.height = std::min(surfCap.maxImageExtent.height,
                                            std::max(getHeight(), surfCap.minImageExtent.height));
        concurrent_frame_count_ = surfCap.minImageCount;

        vk::SwapchainCreateInfoKHR createInfo;
        createInfo.surface = surface_;
        createInfo.minImageCount = concurrent_frame_count_;
        createInfo.imageFormat = surface_format_.format;
        createInfo.imageColorSpace = surface_format_.colorSpace;
        createInfo.imageExtent = swapchain_extent_;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.preTransform = surfCap.currentTransform;
        createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.presentMode = present_mode_;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        swapchain_ = device_.createSwapchainKHR(createInfo);
        swapchain_images_ = device_.getSwapchainImagesKHR(swapchain_);
    }

    void destroy_swapchain() { device_.destroySwapchainKHR(swapchain_); }

    void create_imageviews()
    {
        swapchain_image_views_.resize(swapchain_images_.size());

        for (auto i = 0ul; i < swapchain_image_views_.size(); ++i) {
            vk::ImageViewCreateInfo createInfo;
            createInfo.image = swapchain_images_[i];
            createInfo.viewType = vk::ImageViewType::e2D;
            createInfo.format = surface_format_.format;
            createInfo.components.r = vk::ComponentSwizzle::eIdentity;
            createInfo.components.g = vk::ComponentSwizzle::eIdentity;
            createInfo.components.b = vk::ComponentSwizzle::eIdentity;
            createInfo.components.a = vk::ComponentSwizzle::eIdentity;
            createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            swapchain_image_views_[i] = device_.createImageView(createInfo);
        }
    }

    void destroy_imageviews()
    {
        for (auto &iv : swapchain_image_views_) {
            device_.destroyImageView(iv);
        }
    }

    void create_graphics_command_pool_and_buffer()
    {
        vk::CommandPoolCreateInfo cpci;
        cpci.setQueueFamilyIndex(graphics_queue_index_);
        cpci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        default_command_pool_ = device_.createCommandPool(cpci);

        vk::CommandBufferAllocateInfo cbai;
        cbai.setCommandBufferCount(concurrent_frame_count_);
        cbai.setCommandPool(default_command_pool_);
        cbai.setLevel(vk::CommandBufferLevel::ePrimary);
        default_command_buffers_ = device_.allocateCommandBuffers(cbai);
    }

    void destroy_graphics_command_pool() { device_.destroyCommandPool(default_command_pool_); }

    void create_default_renderpass()
    {
        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
        dependency.setDstSubpass(0);
        dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput
                                   | vk::PipelineStageFlagBits::eEarlyFragmentTests);
        dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
        dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput
                                   | vk::PipelineStageFlagBits::eEarlyFragmentTests);
        dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite
                                    | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

        vk::AttachmentDescription colorAttachment;
        colorAttachment.setFormat(surface_format_.format);
        colorAttachment.setSamples(vk::SampleCountFlagBits::e1);
        colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
        colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
        colorAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        colorAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
        colorAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
        colorAttachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference colorAttachmentRef;
        colorAttachmentRef.setAttachment(0);
        colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::AttachmentDescription depthAttachment;
        depthAttachment.setFormat(depth_format_);
        depthAttachment.setSamples(vk::SampleCountFlagBits::e1);
        depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
        depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);
        depthAttachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        depthAttachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
        depthAttachment.setInitialLayout(vk::ImageLayout::eUndefined);
        depthAttachment.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::AttachmentReference depthAttachmentRef;
        depthAttachmentRef.setAttachment(1);
        depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
        subpass.setColorAttachments(colorAttachmentRef);
        subpass.setPDepthStencilAttachment(&depthAttachmentRef);

        std::array attachmentArray = {colorAttachment, depthAttachment};

        vk::RenderPassCreateInfo rpci;
        rpci.setAttachmentCount(2);
        rpci.setAttachments(attachmentArray);
        rpci.setSubpassCount(1);
        rpci.setSubpasses(subpass);
        rpci.setDependencyCount(1);
        rpci.setDependencies(dependency);

        default_renderpass_ = device_.createRenderPass(rpci);
    }

    void destroy_default_renderpass() { device_.destroyRenderPass(default_renderpass_); }

    void create_framebuffers()
    {
        swapchain_framebuffers_.resize(swapchain_image_views_.size());
        for (auto i = 0ul; i < swapchain_image_views_.size(); ++i) {
            std::array attachments = {swapchain_image_views_[i], depth_image_view_};

            vk::FramebufferCreateInfo fbci;
            fbci.setRenderPass(default_renderpass_);
            fbci.setAttachments(attachments);
            fbci.setWidth(swapchain_extent_.width);
            fbci.setHeight(swapchain_extent_.height);
            fbci.setLayers(1);
            swapchain_framebuffers_[i] = device_.createFramebuffer(fbci);
        }
    }

    void destroy_framebuffers()
    {
        for (auto &f : swapchain_framebuffers_) {
            device_.destroyFramebuffer(f);
        }
    }

    void create_sync_objects()
    {
        image_available_semaphore = device_.createSemaphore({});
        render_finished_semaphore = device_.createSemaphore({});
        vk::FenceCreateInfo fci;
        fci.setFlags(vk::FenceCreateFlagBits::eSignaled);
        in_flight_fence = device_.createFence(fci);
    }

    void destroy_sync_objects()
    {
        device_.destroySemaphore(image_available_semaphore);
        device_.destroySemaphore(render_finished_semaphore);
        device_.destroyFence(in_flight_fence);
    }
};

} // namespace vkopter
