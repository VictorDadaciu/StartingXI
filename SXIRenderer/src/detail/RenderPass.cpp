#include "detail/RenderPass.h"

#include "detail/Context.h"
#include "detail/Window.h"
#include "detail/Utils.h"

#include <iostream>

#include "SXICore/Exception.h"

namespace sxi::renderer::detail
{
    RenderPass* basicRenderPass{};

    RenderPass::RenderPass()
    {
		createRenderPass();
		createResources();
		createFrameBuffers();
    }

    RenderPass::~RenderPass()
    {
		vkDestroyImageView(context->logicalDevice, colorImageView, nullptr);
		vkDestroyImage(context->logicalDevice, colorImage, nullptr);
		vkFreeMemory(context->logicalDevice, colorImageMem, nullptr);

		vkDestroyImageView(context->logicalDevice, depthImageView, nullptr);
		vkDestroyImage(context->logicalDevice, depthImage, nullptr);
		vkFreeMemory(context->logicalDevice, depthImageMem, nullptr);

		for (const VkFramebuffer& framebuffer : frameBuffers)
			vkDestroyFramebuffer(context->logicalDevice, framebuffer, nullptr);

        vkDestroyRenderPass(context->logicalDevice, pass, nullptr);
    }

	void RenderPass::createRenderPass()
	{
		VkSampleCountFlagBits samples = context->currentPhysicalDevice().maxMSAASamples;

        VkAttachmentDescription colorAttachment{};
		colorAttachment.format = window->swapchain->surfaceFormat.format;
		colorAttachment.samples = samples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkFormat depthFormat = findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = samples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = window->swapchain->surfaceFormat.format;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = SXI_TO_U32(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(context->logicalDevice, &renderPassInfo, nullptr, &pass) != VK_SUCCESS)
			throw ResourceCreationException("Failed to create render pass");
	}

	void RenderPass::createResources()
	{
		VkSampleCountFlagBits samples = context->currentPhysicalDevice().maxMSAASamples;
		VkFormat colorFormat = window->swapchain->surfaceFormat.format;
		VkExtent2D extent = window->swapchain->extent;

		createImage(
			extent.width,
			extent.height,
			1,
			samples,
			colorFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			colorImage,
			colorImageMem
		);
		colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

		VkFormat depthFormat = findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
		createImage(
			extent.width,
			extent.height,
			1,
			samples,
			depthFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage,
			depthImageMem
		);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	void RenderPass::createFrameBuffers()
	{
		frameBuffers.resize(window->swapchain->images.size());

		VkExtent2D extent = window->swapchain->extent;

		for (size_t i = 0; i < frameBuffers.size(); ++i)
		{
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				window->swapchain->imageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = pass;
			framebufferInfo.attachmentCount = SXI_TO_U32(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(context->logicalDevice, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
				throw ResourceCreationException("Failed to create framebuffer");
		}
	}
}