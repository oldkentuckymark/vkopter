#pragma once

#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

#include <utility>
#include <map>

namespace vkopter::render
{

class MemoryManager
{
public:
    MemoryManager(vk::Instance inst, vk::Device dev, vk::PhysicalDevice pdev, uint32_t queueFamilyIndex) :
        instance_(inst),
        device_(dev),
        physical_device_(pdev)
    {
        vk::CommandPoolCreateInfo cpci;
        cpci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        cpci.setQueueFamilyIndex(queueFamilyIndex);
        transfer_pool_ = dev.createCommandPool(cpci);

        vk::CommandBufferAllocateInfo cbai;
        cbai.setCommandBufferCount(1);
        cbai.setCommandPool(transfer_pool_);
        cbai.setLevel(vk::CommandBufferLevel::ePrimary);
        transfer_command_buffer_ = device_.allocateCommandBuffers(cbai)[0];


        VmaAllocatorCreateInfo aci = {};
        aci.device = dev;
        aci.physicalDevice = pdev;
        aci.instance = inst;
        aci.vulkanApiVersion = VK_API_VERSION_1_0;
        vmaCreateAllocator(&aci, &allocator_);
        transfer_queue_ = device_.getQueue(queueFamilyIndex,0);
    }

    ~MemoryManager()
    {
        try
        {
        device_.waitIdle();
        }
        catch (...)
        {
            std::abort();
        }

        for(auto& b : buffers_)
        {
            destroyBuffer(b.first);
        }
        device_.destroyCommandPool(transfer_pool_);


        vmaDestroyAllocator(allocator_);



    }

    MemoryManager(MemoryManager& ) = delete;
    MemoryManager(MemoryManager&& ) = delete;
    auto operator = (MemoryManager&) -> MemoryManager& = delete;
    auto operator = (MemoryManager&&) -> MemoryManager& = delete;

    auto createBuffer(vk::BufferUsageFlags const usage, std::size_t const sizeInBytes, void* data = nullptr) -> vk::Buffer
    {

        VkBuffer buffer = {};
        VmaAllocation bufferAllocation = {};
        VmaAllocationInfo bufferAllocationInfo = {};
        VmaAllocationCreateInfo bufferAllocationCreateInfo = {};
        bufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        bufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
        VkBufferCreateInfo bufferCreateInfo = {};
        bufferCreateInfo.pNext = nullptr;
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | (VkBufferUsageFlags)usage;
        bufferCreateInfo.size = sizeInBytes;
        vmaCreateBuffer(allocator_, &bufferCreateInfo, &bufferAllocationCreateInfo, &buffer, &bufferAllocation,&bufferAllocationInfo);

        buffers_.insert({buffer,{bufferAllocation,bufferAllocationInfo}});

        if(data)
        {
            updateBuffer(buffer, 0, sizeInBytes, data);
        }

        return buffer;
    }

    auto updateBuffer(vk::Buffer buffer, vk::DeviceSize offset, vk::DeviceSize const sizeInBytes, void* data) -> void
    {
        if(data == nullptr || sizeInBytes == 0) { return; }

        VkBuffer stagingbuffer = {};
        VmaAllocation stagingbufferAllocation = {};
        VmaAllocationInfo stagingbufferAllocationInfo = {};
        VmaAllocationCreateInfo stagingbufferAllocationCreateInfo = {};
        stagingbufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        stagingbufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VkBufferCreateInfo stagingbufferCreateInfo = {};
        stagingbufferCreateInfo.pNext = nullptr;
        stagingbufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        stagingbufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingbufferCreateInfo.size = sizeInBytes;
        vmaCreateBuffer(allocator_, &stagingbufferCreateInfo, &stagingbufferAllocationCreateInfo, &stagingbuffer, &stagingbufferAllocation,&stagingbufferAllocationInfo);

        void* memory = nullptr;

        vmaInvalidateAllocation(allocator_, stagingbufferAllocation, offset, sizeInBytes);
        vmaMapMemory(allocator_, stagingbufferAllocation, &memory);
        std::memcpy(static_cast<char*>(memory)+offset, data, sizeInBytes);
        vmaUnmapMemory(allocator_, stagingbufferAllocation);
        vmaFlushAllocation(allocator_, stagingbufferAllocation, offset, sizeInBytes);


        vk::CommandBufferBeginInfo cbbi;
        cbbi.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        transfer_command_buffer_.begin(cbbi);
        transfer_command_buffer_.copyBuffer(stagingbuffer, buffer, vk::BufferCopy{0,offset,sizeInBytes});
        transfer_command_buffer_.end();

        vk::SubmitInfo si;
        si.setCommandBuffers(transfer_command_buffer_);
        transfer_queue_.submit(si);
        transfer_queue_.waitIdle();


        vmaDestroyBuffer(allocator_,stagingbuffer, stagingbufferAllocation);

    }

    auto destroyBuffer(vk::Buffer buffer) -> void
    {
        vmaDestroyBuffer(allocator_, buffer, buffers_[buffer].first);
        buffers_.erase(buffer);
    }

    auto createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage, void* data = nullptr) -> vk::Image
    {
        VkImage img = VK_NULL_HANDLE;
        VkImageCreateInfo ici = {};

        VmaAllocation imageAllocation = {};
        VmaAllocationInfo imageAllocationInfo = {};
        VmaAllocationCreateInfo imageAllocationCreateInfo = {};
        imageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        imageAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;

        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.pNext = nullptr;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.extent.width = width;
        ici.extent.height = height;
        ici.extent.depth = 1;
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.format = VkFormat(format);;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ici.usage = VkImageUsageFlags(usage);
        ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.flags = 0;



        auto r = vmaCreateImage(allocator_, &ici, &imageAllocationCreateInfo, &img, &imageAllocation, &imageAllocationInfo);
        images_.insert({img,{imageAllocation,imageAllocationInfo}});



        if(data)
        {
            int offset = 0;
            int sizeInBytes = width * height * 4;


            //create staging buufer with pixel data inside
            VkBuffer stagingbuffer = {};
            VmaAllocation stagingbufferAllocation = {};
            VmaAllocationInfo stagingbufferAllocationInfo = {};
            VmaAllocationCreateInfo stagingbufferAllocationCreateInfo = {};
            stagingbufferAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            stagingbufferAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            VkBufferCreateInfo stagingbufferCreateInfo = {};
            stagingbufferCreateInfo.pNext = nullptr;
            stagingbufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingbufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            stagingbufferCreateInfo.size = sizeInBytes;
            vmaCreateBuffer(allocator_, &stagingbufferCreateInfo, &stagingbufferAllocationCreateInfo, &stagingbuffer, &stagingbufferAllocation,&stagingbufferAllocationInfo);

            void* memory = nullptr;

            vmaInvalidateAllocation(allocator_, stagingbufferAllocation, offset, sizeInBytes);
            vmaMapMemory(allocator_, stagingbufferAllocation, &memory);
            std::memcpy(static_cast<char*>(memory)+offset, data, sizeInBytes);
            vmaUnmapMemory(allocator_, stagingbufferAllocation);
            vmaFlushAllocation(allocator_, stagingbufferAllocation, offset, sizeInBytes);


            vk::CommandBufferBeginInfo cbbi;
            cbbi.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
            transfer_command_buffer_.begin(cbbi);


            //copy staging buffer to image
            vk::ImageSubresourceRange isrr = {};
            isrr.aspectMask = vk::ImageAspectFlagBits::eColor;
            isrr.baseMipLevel = 0;
            isrr.levelCount = 1;
            isrr.baseArrayLayer = 0;
            isrr.layerCount = 1;

            //transfer image to transferdst layout using a pipeline barrier
            vk::ImageMemoryBarrier imbTransfer = {};
            imbTransfer.oldLayout = vk::ImageLayout::eUndefined;
            imbTransfer.newLayout = vk::ImageLayout::eTransferDstOptimal;
            imbTransfer.image = img;
            imbTransfer.subresourceRange = isrr;
            imbTransfer.srcAccessMask = vk::AccessFlagBits::eNone;
            imbTransfer.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            imbTransfer.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            imbTransfer.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

            transfer_command_buffer_.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {},{},imbTransfer);


            //copy pixel data to image from staging buffer
            vk::BufferImageCopy bic = {};
            bic.bufferOffset = 0;
            bic.bufferRowLength = 0;
            bic.bufferImageHeight = height;

            bic.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bic.imageSubresource.mipLevel = 0;
            bic.imageSubresource.baseArrayLayer = 0;
            bic.imageSubresource.layerCount = 0;
            bic.imageExtent = vk::Extent3D{width,height,1};

            transfer_command_buffer_.copyBufferToImage(stagingbuffer,img,vk::ImageLayout::eTransferDstOptimal,bic);

            //transition image layout to shader readonly optimal
            vk::ImageMemoryBarrier imbReadable = {};
            imbReadable.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            imbReadable.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            imbReadable.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            imbReadable.dstAccessMask = vk::AccessFlagBits::eShaderRead;
            imbReadable.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
            imbReadable.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
            imbReadable.image = img;
            imbReadable.subresourceRange = isrr;


            transfer_command_buffer_.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {},{},imbTransfer);

            transfer_command_buffer_.end();

            vk::SubmitInfo si;
            si.setCommandBuffers(transfer_command_buffer_);
            transfer_queue_.submit(si);

            transfer_queue_.waitIdle();

            vmaDestroyBuffer(allocator_,stagingbuffer, stagingbufferAllocation);
        }



        return img;
    }

    auto destroyImage(vk::Image image) -> void
    {
        auto i = VkImage(image);
        vmaDestroyImage(allocator_, i, images_[image].first);
        images_.erase(image);
    }

private:
    VmaAllocator allocator_ = nullptr;
    vk::Instance instance_;
    vk::Device device_;
    vk::PhysicalDevice physical_device_;
    vk::CommandPool transfer_pool_;
    vk::CommandBuffer transfer_command_buffer_;
    vk::Queue transfer_queue_;

    std::map<vk::Buffer, std::pair<VmaAllocation, VmaAllocationInfo>> buffers_;
    std::map<vk::Image, std::pair<VmaAllocation, VmaAllocationInfo>> images_;
};

}
