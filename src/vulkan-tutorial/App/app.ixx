export module vulkantutorial:app;
import std;
import :libs;
import :error;
import :util;
import :vulkanite;
import :glm;

export namespace VulkanTutorial::App
{
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;

		static auto ToBindingDescription() -> vk::VertexInputBindingDescription
		{
			return vk::VertexInputBindingDescription{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vk::VertexInputRate::eVertex
			};
		}

		static auto GetAttributeDescriptions() -> std::array<vk::VertexInputAttributeDescription, 2>
		{
			return {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, Util::OffsetOf(&Vertex::pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, Util::OffsetOf(&Vertex::color))
			};
		}
	};

	const std::vector<Vertex> vertices = {
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	constexpr auto Width = std::uint32_t{ 800 };
	constexpr auto Height = std::uint32_t{ 600 };
	constexpr int MaxFramesInFlight = 2;

	class MainApp
	{
	public:
		constexpr MainApp() = default;
		// Not copyable or movable
		MainApp(const MainApp&) = delete;
		auto operator=(this MainApp&, const MainApp&) -> MainApp& = delete;

		void Run(this MainApp& self)
		{
			self.InitWindow()
				.InitVulkan()
				.MainLoop()
				.Cleanup();
		}

	private: // Private fields
		glfw::GLFWwindow* window;
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
		Vulkanite::Device::PhysicalDevice physicalDevice;
		Vulkanite::Device::LogicalDevice device = nullptr;
		vk::raii::SurfaceKHR surface = nullptr;
		constexpr static auto deviceExtensions = std::array{ vk::KHRSwapchainExtensionName };
		vk::raii::Queue queue = nullptr;
		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		vk::SurfaceFormatKHR swapChainSurfaceFormat;
		vk::Extent2D swapChainExtent;
		std::uint32_t graphicsFamily = 0; // official tutorial misses these
		std::uint32_t presentFamily = 0;
		std::vector<vk::raii::ImageView> swapChainImageViews;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;
		vk::raii::CommandPool commandPool = nullptr;

		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		std::vector<vk::raii::Fence> inFlightFences;
		std::uint32_t frameIndex = 0;
		bool framebufferResized = false;

	private: // Core internal initialisation methods.
		// The first step is to initialise the GLFW window.
		auto InitWindow(this MainApp& self) -> MainApp&
		{
			glfw::glfwInit();

			glfw::glfwWindowHint(glfw::ClientApi, glfw::NoApi);
			glfw::glfwWindowHint(glfw::Resizable, false);

			self.window = glfw::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
			glfw::glfwSetWindowUserPointer(self.window, &self);
			glfw::glfwSetFramebufferSizeCallback(self.window, FramebufferResizeCallback);

			return self;
		}

		// We then need to initialise our connection 
		// to Vulkan by creating a Vulkan instance.
		auto InitVulkan(this MainApp& self) -> MainApp&
		{
			self.CreateInstance();
			self.SetupDebugMessenger();
			self.CreateSurface();
			self.PickPhysicalDevice();
			self.CreateLogicalDevice();
			self.CreateSwapChain();
			self.CreateImageViews();
			self.CreateGraphicsPipeline();
			self.CreateCommandPool();
			self.CreateVertexBuffer();
			self.CreateCommandBuffers();
			self.CreateSyncObjects();
			return self;
		}

		auto MainLoop(this MainApp& self) -> MainApp&
		{
			while (not glfw::glfwWindowShouldClose(self.window))
			{
				glfw::glfwPollEvents();
				self.DrawFrame();
			}
			self.device->waitIdle();
			return self;
		}

	private: // Internal methods.
		vk::raii::Buffer       vertexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;

		auto FindMemoryType(
			this auto&& self, 
			std::uint32_t typeFilter, 
			vk::MemoryPropertyFlags properties
		) -> std::uint32_t
		{
			auto memProperties = vk::PhysicalDeviceMemoryProperties{ self.physicalDevice->getMemoryProperties() };

			for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if ((typeFilter & (1 << i)) and (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			throw std::runtime_error("failed to find suitable memory type!");
		}

		auto CreateVertexBuffer(this MainApp& self) -> decltype(self)
		{
			auto bufferInfo = vk::BufferCreateInfo{ 
				.size = sizeof(vertices[0]) * vertices.size(), 
				.usage = vk::BufferUsageFlagBits::eVertexBuffer, 
				.sharingMode = vk::SharingMode::eExclusive 
			};
			self.vertexBuffer = vk::raii::Buffer(self.device, bufferInfo);

			auto memRequirements = vk::MemoryRequirements{ self.vertexBuffer.getMemoryRequirements() };
			auto memoryProperties = vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			auto memoryAllocateInfo = vk::MemoryAllocateInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = self.FindMemoryType(memRequirements.memoryTypeBits, memoryProperties) };
			self.vertexBufferMemory = vk::raii::DeviceMemory(self.device, memoryAllocateInfo);
			self.vertexBuffer.bindMemory(*self.vertexBufferMemory, 0);

			void* data = self.vertexBufferMemory.mapMemory(0, bufferInfo.size);
			std::memcpy(data, vertices.data(), bufferInfo.size);
			self.vertexBufferMemory.unmapMemory();

			return self;
		}

		static void FramebufferResizeCallback(glfw::GLFWwindow* window, int width, int height)
		{
			auto app = reinterpret_cast<MainApp*>(glfw::glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		}

		void RecreateSwapChain(this MainApp& self)
		{
			int width = 0, height = 0;
			glfwGetFramebufferSize(self.window, &width, &height);
			while (width == 0 or height == 0)
			{
				glfwGetFramebufferSize(self.window, &width, &height);
				glfw::glfwWaitEvents();
			}

			self.device->waitIdle();

			self.CleanupSwapChain();
			self.CreateSwapChain();
			self.CreateImageViews();
		}

		void CleanupSwapChain(this MainApp& self)
		{
			self.swapChainImageViews.clear();
			self.swapChain = nullptr;
		}

		void RecordCommandBuffer(this MainApp& self, uint32_t imageIndex)
		{
			auto& commandBuffer = self.commandBuffers[self.frameIndex];
			commandBuffer.begin({});
			// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
			self.TransitionImageLayout(
				imageIndex,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eColorAttachmentOptimal,
				{},                                                        // srcAccessMask (no need to wait for previous operations)
				vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
				vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
			);
			auto clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
			auto attachmentInfo = vk::RenderingAttachmentInfo{
				.imageView = self.swapChainImageViews[imageIndex],
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = clearColor };
			auto renderingInfo = vk::RenderingInfo{
				.renderArea = {.offset = {0, 0}, .extent = self.swapChainExtent},
				.layerCount = 1,
				.colorAttachmentCount = 1,
				.pColorAttachments = &attachmentInfo };

			commandBuffer.beginRendering(renderingInfo);
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *self.graphicsPipeline);
			commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(self.swapChainExtent.width), static_cast<float>(self.swapChainExtent.height), 0.0f, 1.0f));
			commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), self.swapChainExtent));
			commandBuffer.bindVertexBuffers(0, *self.vertexBuffer, { 0 });
			commandBuffer.draw(3, 1, 0, 0);
			commandBuffer.endRendering();
			// After rendering, transition the swapchain image to PRESENT_SRC
			self.TransitionImageLayout(
				imageIndex,
				vk::ImageLayout::eColorAttachmentOptimal,
				vk::ImageLayout::ePresentSrcKHR,
				vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
				{},                                                        // dstAccessMask
				vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
				vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
			);
			commandBuffer.end();
		}

		void DrawFrame(this MainApp& self)
		{
			auto fenceResult = self.device->waitForFences(*self.inFlightFences[self.frameIndex], true, std::numeric_limits<std::uint64_t>::max());
			if (fenceResult != vk::Result::eSuccess)
			{
				throw std::runtime_error("failed to wait for fence!");
			}

			auto [result, imageIndex] = self.swapChain.acquireNextImage(std::numeric_limits<std::uint64_t>::max(), *self.presentCompleteSemaphores[self.frameIndex], nullptr);
			if (result == vk::Result::eErrorOutOfDateKHR)
			{
				self.RecreateSwapChain();
				return;
			}
			if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
				throw std::runtime_error("failed to acquire swap chain image!");

			self.device->resetFences(*self.inFlightFences[self.frameIndex]);
			self.commandBuffers[self.frameIndex].reset();
			self.RecordCommandBuffer(imageIndex);

			auto waitDestinationStageMask = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			const auto submitInfo = vk::SubmitInfo{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*self.presentCompleteSemaphores[self.frameIndex],
				.pWaitDstStageMask = &waitDestinationStageMask,
				.commandBufferCount = 1,
				.pCommandBuffers = &*self.commandBuffers[self.frameIndex],
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = &*self.renderFinishedSemaphores[imageIndex]
			};
			self.queue.submit(submitInfo, *self.inFlightFences[self.frameIndex]);

			const auto presentInfoKHR = vk::PresentInfoKHR{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*self.renderFinishedSemaphores[imageIndex],
				.swapchainCount = 1,
				.pSwapchains = &*self.swapChain,
				.pImageIndices = &imageIndex
			};

			result = self.queue.presentKHR(presentInfoKHR);

			if ((result == vk::Result::eSuboptimalKHR) 
				or (result == vk::Result::eErrorOutOfDateKHR) 
				or self.framebufferResized)
			{
				self.framebufferResized = false;
				self.RecreateSwapChain();
			}
			else if (result != vk::Result::eSuccess)
			{
				// There are no other success codes than eSuccess; on any error code, presentKHR already threw an exception.
				std::abort();
			}
			self.frameIndex = (self.frameIndex + 1) % MaxFramesInFlight;
		}

		// Before we can start rendering to an image, we need to 
		// transition its layout to one that is suitable for 
		// rendering. In Vulkan, images can be in different 
		// layouts that are optimized for different operations. 
		// For example, an image can be in a layout that is 
		// optimal for presenting to the screen, or in a layout 
		// that is optimal for being used as a color attachment.
		void TransitionImageLayout(
			this MainApp& self,
			uint32_t imageIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask
		)
		{
			auto barrier = vk::ImageMemoryBarrier2{
				.srcStageMask = srcStageMask,
				.srcAccessMask = srcAccessMask,
				.dstStageMask = dstStageMask,
				.dstAccessMask = dstAccessMask,
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.srcQueueFamilyIndex = Vulkan::QueueFamilyIgnored,
				.dstQueueFamilyIndex = Vulkan::QueueFamilyIgnored,
				.image = self.swapChainImages[imageIndex],
				.subresourceRange = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			auto dependencyInfo = vk::DependencyInfo{
				.dependencyFlags = {},
				.imageMemoryBarrierCount = 1,
				.pImageMemoryBarriers = &barrier
			};
			self.commandBuffers[self.frameIndex].pipelineBarrier2(dependencyInfo);
		}

		void CreateSyncObjects(this MainApp& self)
		{
			for (size_t i = 0; i < self.swapChainImages.size(); i++)
			{
				self.renderFinishedSemaphores.emplace_back(self.device, vk::SemaphoreCreateInfo());
			}

			for (size_t i = 0; i < MaxFramesInFlight; i++)
			{
				self.presentCompleteSemaphores.emplace_back(self.device, vk::SemaphoreCreateInfo());
				self.inFlightFences.emplace_back(self.device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
			}
		}

		// Commands in Vulkan, like drawing operations and memory transfers, 
		// are not executed directly using function calls. You have to 
		// record all the operations you want to perform in command buffer 
		// objects. The advantage of this is that when we are ready to tell 
		// Vulkan what we want to do, all the commands are submitted 
		// together. We have to create a command pool before we can create 
		// command buffers. Command pools manage the memory that is used to 
		// store the buffers and command buffers are allocated from them.
		void CreateCommandPool(this MainApp& self)
		{
			auto poolInfo = vk::CommandPoolCreateInfo{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = self.graphicsFamily 
			};
			self.commandPool = vk::raii::CommandPool(self.device, poolInfo);
		}

		void CreateCommandBuffers(this MainApp& self)
		{
			// VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue 
			// for execution, but cannot be called from other command buffers.
			// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted 
			// directly, but can be called from primary command buffers.
			vk::CommandBufferAllocateInfo allocInfo{ 
				.commandPool = *self.commandPool, 
				.level = vk::CommandBufferLevel::ePrimary, 
				.commandBufferCount = MaxFramesInFlight 
			};
			self.commandBuffers = vk::raii::CommandBuffers(self.device, allocInfo);
		}

		// We must describe the graphics pipeline to Vulkan, so Vulkan knows how to best optimize it.
		// Remember, that the pipeline is essentially immutable once created.
		void CreateGraphicsPipeline(this MainApp& self)
		{
			auto shaderModule = Vulkanite::Shaders::ShaderModule{
				self.device.CreateShaderModule("shaders/slang.spv")
			};

			// Here, we're binding the shader modules to the pipeline stages.
			auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo{
				.stage = vk::ShaderStageFlagBits::eVertex, // Vertex shader stage
				.module = shaderModule.Get(),
				.pName = "vertMain" 
			};
			auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo{
				.stage = vk::ShaderStageFlagBits::eFragment, // Fragment shader stage
				.module = shaderModule.Get(),
				.pName = "fragMain" 
			};
			auto shaderStages = std::array{ vertShaderStageInfo, fragShaderStageInfo };

			auto bindingDescription = Vertex::ToBindingDescription();
			auto attributeDescriptions = Vertex::GetAttributeDescriptions();
			auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo{
				.vertexBindingDescriptionCount = 1,
				.pVertexBindingDescriptions = &bindingDescription,
				.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
				.pVertexAttributeDescriptions = attributeDescriptions.data() 
			};
			// Describes what kind of geometry will be drawn from the vertices and 
			// if primitive restart should be enabled.
			auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo{
				.topology = vk::PrimitiveTopology::eTriangleList 
			};
			// A viewport basically describes the region of the framebuffer 
			// that the output will be rendered to. This will almost always 
			// be (0, 0) to (width, height). While viewports define the 
			// transformation from the image to the framebuffer, scissor 
			// rectangles define in which region pixels will actually be 
			// stored. The rasterizer will discard any pixels outside the 
			// scissored rectangles. They function like a filter rather 
			// than a transformation. Viewport(s) and scissor rectangle(s) 
			// can either be specified as a static part of the pipeline or 
			// as a dynamic state set in the command buffer, which is the 
			// norm now.
			auto viewportState = vk::PipelineViewportStateCreateInfo{
				.viewportCount = 1, 
				.scissorCount = 1 
			};
			// The rasterizer takes the geometry shaped by the vertices 
			// from the vertex shader and turns it into fragments to be 
			// colored by the fragment shader. It also performs depth 
			// testing, face culling and the scissor test, and it can 
			// be configured to output fragments that fill entire 
			// polygons or just the edges (wireframe rendering).
			auto rasterizer = vk::PipelineRasterizationStateCreateInfo{
				.depthClampEnable = false, 
				.rasterizerDiscardEnable = false, 
				.polygonMode = vk::PolygonMode::eFill, 
				.cullMode = vk::CullModeFlagBits::eBack, 
				.frontFace = vk::FrontFace::eClockwise, 
				.depthBiasEnable = false, 
				.depthBiasSlopeFactor = 1.0f, 
				.lineWidth = 1.0f 
			};
			// The VkPipelineMultisampleStateCreateInfo struct configures 
			// multisampling, which is one of the ways to perform 
			// antialiasing. It works by combining the fragment shader 
			// results of multiple polygons that rasterize to the same 
			// pixel.
			auto multisampling = vk::PipelineMultisampleStateCreateInfo{
				.rasterizationSamples = vk::SampleCountFlagBits::e1, 
				.sampleShadingEnable = false
			};

			constexpr auto colorWriteMask = 
				[](auto...components) static constexpr->std::uint32_t
				{
					return (static_cast<std::uint32_t>(components) | ...);
				}(vk::ColorComponentFlagBits::eR, vk::ColorComponentFlagBits::eG, vk::ColorComponentFlagBits::eB, vk::ColorComponentFlagBits::eA);
			// After a fragment shader has returned a color, it needs 
			// to be combined with the color that is already in the 
			// framebuffer.This transformation is known as color 
			// blending, and there are two ways to do it: Mix the old 
			// and new value to produce a final color, or combine the 
			// old and new value using a bitwise operation. There are 
			// two types of structs to configure color blending.The 
			// first struct, VkPipelineColorBlendAttachmentState 
			// contains the configuration per attached framebuffer 
			// and the second struct, 
			// VkPipelineColorBlendStateCreateInfo contains the 
			// global color blending settings
			auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState{
				.blendEnable = false,
				.colorWriteMask = vk::ColorComponentFlags{colorWriteMask}
			};
			auto colorBlending = vk::PipelineColorBlendStateCreateInfo{
				.logicOpEnable = false, 
				.logicOp = vk::LogicOp::eCopy, 
				.attachmentCount = 1, 
				.pAttachments = &colorBlendAttachment 
			};

			// While mostly immutable, some pipeline state can be flagged
			// as recreatable without recreating the pipeline. This is 
			// done via dynamic states and the dynamic state create info.
			// This data is supplied at drawing time and this is often 
			// done for viewport and scissor state.
			constexpr auto dynamicStates = std::array{
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor 
			};
			vk::PipelineDynamicStateCreateInfo dynamicState{ 
				.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()), 
				.pDynamicStates = dynamicStates.data() 
			};

			vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
			self.pipelineLayout = vk::raii::PipelineLayout(self.device, pipelineLayoutInfo);

			// Dynamic rendering simplifies the rendering process by 
			// eliminating the need for render pass and framebuffer objects.
			// To use dynamic rendering, we need to specify the formats of the 
			// attachments that will be used during rendering. This is done 
			// through the vk::PipelineRenderingCreateInfo structure when 
			// creating the graphics pipeline.
			using StructureChain = vk::StructureChain<
				vk::GraphicsPipelineCreateInfo,
				vk::PipelineRenderingCreateInfo>;
			auto pipelineCreateInfoChain = StructureChain{
				{
					.stageCount = 2,
					.pStages = shaderStages.data(),
					.pVertexInputState = &vertexInputInfo,
					.pInputAssemblyState = &inputAssembly,
					.pViewportState = &viewportState,
					.pRasterizationState = &rasterizer,
					.pMultisampleState = &multisampling,
					.pColorBlendState = &colorBlending,
					.pDynamicState = &dynamicState,
					.layout = self.pipelineLayout,
					.renderPass = nullptr // renderpass-less
				},
				{
					.colorAttachmentCount = 1, 
					.pColorAttachmentFormats = &self.swapChainSurfaceFormat.format
				}
			};

			self.graphicsPipeline = vk::raii::Pipeline(self.device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
		}

		[[nodiscard]] 
		auto CreateShaderModule(
			this MainApp& self, 
			const std::vector<std::byte>& code
		) -> vk::raii::ShaderModule 
		{
			auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo{
				.codeSize = code.size(),
				.pCode = reinterpret_cast<const uint32_t*>(code.data())
			};
			return vk::raii::ShaderModule(self.device, shaderModuleCreateInfo);
		}

		void CreateImageViews(this MainApp& self)
		{
			self.swapChainImageViews.clear();
			auto imageViewCreateInfo = vk::ImageViewCreateInfo{
				.viewType = vk::ImageViewType::e2D, 
				.format = self.swapChainSurfaceFormat.format,
				.components { 
					.r = vk::ComponentSwizzle::eIdentity,
					.g = vk::ComponentSwizzle::eIdentity,
					.b = vk::ComponentSwizzle::eIdentity,
					.a = vk::ComponentSwizzle::eIdentity,
				},
				.subresourceRange { 
					.aspectMask = vk::ImageAspectFlagBits::eColor, 
					.baseMipLevel = 0,
					.levelCount = 1, 
					.baseArrayLayer = 0, 
					.layerCount = 1 
				}
			};
			for (const vk::Image& image : self.swapChainImages)
			{
				imageViewCreateInfo.image = image;
				self.swapChainImageViews.emplace_back(self.device, imageViewCreateInfo);
			}
		}

		static auto ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities) -> uint32_t
		{
			auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
			if (0 < surfaceCapabilities.maxImageCount) 
				if (surfaceCapabilities.maxImageCount < minImageCount)
					minImageCount = surfaceCapabilities.maxImageCount;
			return minImageCount;
		}

		void CreateSwapChain(this MainApp& self)
		{
			auto surfaceCapabilities = vk::SurfaceCapabilitiesKHR{ self.physicalDevice->getSurfaceCapabilitiesKHR(self.surface) };
			self.swapChainExtent = self.ChooseSwapExtent(surfaceCapabilities);
			self.swapChainSurfaceFormat = self.ChooseSwapSurfaceFormat(self.physicalDevice->getSurfaceFormatsKHR(self.surface));

			auto swapChainCreateInfo = vk::SwapchainCreateInfoKHR{
				.flags = vk::SwapchainCreateFlagsKHR(),
				.surface = self.surface,
				.minImageCount = self.ChooseSwapMinImageCount(surfaceCapabilities),
				.imageFormat = self.swapChainSurfaceFormat.format,
				.imageColorSpace = self.swapChainSurfaceFormat.colorSpace,
				.imageExtent = self.swapChainExtent,
				.imageArrayLayers = 1,
				.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
				.imageSharingMode = vk::SharingMode::eExclusive,
				.preTransform = surfaceCapabilities.currentTransform,
				.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
				.presentMode = self.ChooseSwapPresentMode(self.physicalDevice->getSurfacePresentModesKHR(self.surface)),
				.clipped = true,
				.oldSwapchain = nullptr
			};
			self.swapChain = vk::raii::SwapchainKHR(self.device, swapChainCreateInfo);
			self.swapChainImages = self.swapChain.getImages();
		}

		void CreateSurface(this MainApp& self)
		{
			auto surface = Vulkan::VkSurfaceKHR{};
			auto result = vk::Result{ glfw::glfwCreateWindowSurface(*self.instance, self.window, nullptr, &surface) };
			if (result != vk::Result::eSuccess)
				throw Error::VulkanError("Failed to create window surface.");
			self.surface = vk::raii::SurfaceKHR{ self.instance, surface };
		}

		auto ChooseSwapPresentMode(
			this MainApp&,
			const std::vector<vk::PresentModeKHR>& availablePresentModes
		) -> vk::PresentModeKHR
		{
			// Only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available.
			// VK_PRESENT_MODE_FIFO_KHR is more important for mobile devices,
			// where energy usage matters.
			for (const auto& availablePresentMode : availablePresentModes) 
				if (availablePresentMode == vk::PresentModeKHR::eMailbox)
					return availablePresentMode;
			return vk::PresentModeKHR::eFifo;
		}

		// The swap extent is the resolution of the swap chain images
		auto ChooseSwapExtent(
			this MainApp& self, 
			const vk::SurfaceCapabilitiesKHR& capabilities
		) -> vk::Extent2D
		{
			if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
				return capabilities.currentExtent;
			
			int width{}, height{};
			glfw::glfwGetFramebufferSize(self.window, &width, &height);
			return { 
				std::clamp<std::uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
				std::clamp<std::uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		}

		void CreateLogicalDevice(this MainApp& self)
		{
			auto graphicsIndex = std::optional{ self.physicalDevice.FindGraphicsQueueFamilyIndex() };
			if (not graphicsIndex)
				throw Error::VulkanError("Failed to find graphics queue family index.");
			
			auto presentIndex = std::optional{ self.physicalDevice.FindPresentQueueFamilyIndexForSurface(self.surface) };
			if (not presentIndex)
				throw Error::VulkanError("Failed to find present queue family index.");

			self.graphicsFamily = *graphicsIndex;
			self.presentFamily = *presentIndex;

			// The tutorial is weird and uses a structure chain in
			// one page https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/04_Logical_device_and_queues.html
			// and then forgets about it in a subsequent link: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/00_Window_surface.html.
			using StructureChain = vk::StructureChain<
				vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan11Features,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
			>;
			auto featureChain = StructureChain{
				{},
				{.shaderDrawParameters = true},
				{.synchronization2 = true, .dynamicRendering = true},
				{.extendedDynamicState = true}
			};

			auto queuePriority = float{ 0.5f };
			auto deviceQueueCreateInfo = vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = *graphicsIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};

			// Previous versions of Vulkan made a distinction between
			// instance and device validation layers, but this is no 
			// longer the case, and the associated members of the 
			// struct are now ignored by newer implementations.
			auto deviceCreateInfo = vk::DeviceCreateInfo{
				.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &deviceQueueCreateInfo,
				.enabledExtensionCount = static_cast<std::uint32_t>(deviceExtensions.size()),
				.ppEnabledExtensionNames = deviceExtensions.data()
			};
			self.device = 
				self.physicalDevice.CreateLogicalDevice(deviceCreateInfo);
			self.queue = self.device.GetQueue(*graphicsIndex, 0);

			auto surfaceCapabilities = std::vector<vk::SurfaceCapabilitiesKHR>{
				self.physicalDevice->getSurfaceCapabilitiesKHR(self.surface)
			};
			auto availableFormats = std::vector<vk::SurfaceFormatKHR>{
				self.physicalDevice->getSurfaceFormatsKHR(self.surface)
			};
			auto availablePresentModes = std::vector<vk::PresentModeKHR>{
				self.physicalDevice->getSurfacePresentModesKHR(self.surface)
			};
		}

		auto ChooseSwapSurfaceFormat(
			const std::vector<vk::SurfaceFormatKHR>& availableFormats
		) -> vk::SurfaceFormatKHR
		{
			if (availableFormats.empty())
				throw Error::VulkanError("No available surface formats found.");

			for (const auto& availableFormat : availableFormats) 
				if (availableFormat.format == vk::Format::eB8G8R8A8Srgb)
					if (availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
						return availableFormat;
			return availableFormats[0];
		}

		void PickPhysicalDevice(this MainApp& self)
		{
			auto physicalDevices = self.instance.enumeratePhysicalDevices();
			if (physicalDevices.empty())
				throw Error::VulkanError("Failed to find GPUs with Vulkan support.");

			auto deviceList = Vulkanite::Device::PhysicalDeviceList{ physicalDevices };
			deviceList.FilterUnsupportedDevices();
			std::println("{}", deviceList);
			
			std::optional supported = deviceList.FirstSupportedDevice();
			if (not supported)
				throw Error::VulkanError("Failed to find a suitable GPU.");

			auto bestDevice = Vulkanite::Device::ScoredPhysicalDevice{ *std::move(supported) };
			std::println("Selected physical device: {}", bestDevice);
			self.physicalDevice = std::move(bestDevice).ToGraphicsProcessingUnit();
		}

		void Cleanup(this MainApp& self)
		{
			self.CleanupSwapChain();
			glfw::glfwDestroyWindow(self.window);
			glfw::glfwTerminate();
		}

		void PrintSupportedExtensions(this const MainApp& self)
		{
			auto availableExtensions =
				std::vector<vk::ExtensionProperties>{ self.context.enumerateInstanceExtensionProperties() };
			std::println("Available Vulkan extensions ({}):", availableExtensions.size());
			for (const auto& ext : availableExtensions)
				std::println(" -> {}", std::string_view{ ext.extensionName });
		}

		auto GetRequiredVulkanExtensions(this const MainApp& self) -> std::vector<std::string>
		{
			// We need glfw to tell us what extensions to use.
			auto glfwExtensionCount = std::uint32_t{ 0 };
			auto glfwExtensions = glfw::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			// Is the extension requested by glfw supported by vulkan?
			auto supportedExtensions = self.context.enumerateInstanceExtensionProperties();
			// Treat argv as a span of const char*
			auto requiredExtensions =
				std::span<const char* const>{ glfwExtensions, glfwExtensionCount }
			| std::ranges::to<std::vector<std::string>>();
			if (Util::EnableValidationLayers)
				requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
			for (const std::string_view extName : requiredExtensions)
			{
				bool supported = std::ranges::any_of(
					supportedExtensions,
					[extName](const vk::ExtensionProperties& extProp)
					{
						return extName == extProp.extensionName;
					}
				);
				if (not supported)
					throw Error::VulkanError(std::format("Required extension not supported: {}", extName));
			}
			return requiredExtensions;
		}

		static auto DebugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
			vk::DebugUtilsMessageTypeFlagsEXT type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void*
		) -> vk::Bool32
		{
			std::println("A");
			if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
				std::println("Validation layer: {}", pCallbackData->pMessage);
			return false;
		}

		auto GetRequiredVulkanLayers(this MainApp& self) -> std::vector<std::string>
		{
			auto requiredLayers = std::vector<std::string>{
				//"VK_LAYER_LUNARG_api_dump" 
			};
			if constexpr (Util::EnableValidationLayers)
			{
				requiredLayers.push_back("VK_LAYER_KHRONOS_validation");
				//layers.push_back("VK_LAYER_LUNARG_api_dump");
			}

			auto layerProperties = self.context.enumerateInstanceLayerProperties();
			for (std::string_view requiredLayer : requiredLayers)
			{
				bool supported = std::ranges::any_of(
					layerProperties,
					[requiredLayer](const vk::LayerProperties& layerProp)
					{
						return requiredLayer == layerProp.layerName;
					}
				);
				if (not supported)
					throw Error::VulkanError(std::format("Required layer not supported: {}", requiredLayer));
			}

			return requiredLayers;
		}

		void CreateInstance(this MainApp& self)
		{
			self.PrintSupportedExtensions();

			// Check required layers are supported.
			auto requiredLayers = self.GetRequiredVulkanLayers();
			auto requiredExtensions = self.GetRequiredVulkanExtensions();
			// Transform required layers and extensions to const char* arrays
			// for InstanceCreateInfo().
			auto layersList = requiredLayers
				| std::ranges::views::transform([](const std::string& layer) { return layer.c_str(); })
				| std::ranges::to<std::vector<const char*>>();
			auto extensionList = requiredExtensions
				| std::ranges::views::transform([](auto&& s) { return s.c_str(); })
				| std::ranges::to<std::vector<const char*>>();

			// Create the Vulkan instance
			constexpr auto appInfo = vk::ApplicationInfo{
				.pApplicationName = "Hello Triangle",
				.applicationVersion = vk::MakeVersion(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = vk::MakeVersion(1, 0, 0),
				.apiVersion = vk::ApiVersion14
			};
			auto createInfo = vk::InstanceCreateInfo{
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = static_cast<std::uint32_t>(layersList.size()),
				.ppEnabledLayerNames = layersList.data(),
				.enabledExtensionCount = static_cast<std::uint32_t>(extensionList.size()),
				.ppEnabledExtensionNames = extensionList.data(),
			};
			self.instance = vk::raii::Instance(self.context, createInfo);
		}

		void SetupDebugMessenger(this MainApp& self)
		{
			if (not Util::EnableValidationLayers)
				return;

			constexpr auto ToUint32 =
				[](auto...v) static constexpr noexcept -> std::uint32_t
				{
					return (static_cast<std::uint32_t>(v) | ...);
				};
			using Severities = vk::DebugUtilsMessageSeverityFlagBitsEXT;
			auto severities = 
				ToUint32(Severities::eVerbose, Severities::eWarning, Severities::eError);
			auto severityFlags = vk::DebugUtilsMessageSeverityFlagsEXT(severities);

			using MessageTypes = vk::DebugUtilsMessageTypeFlagBitsEXT;
			auto messageTypes = 
				ToUint32(MessageTypes::eGeneral, MessageTypes::ePerformance, MessageTypes::eValidation);
			auto messageTypeFlags = vk::DebugUtilsMessageTypeFlagsEXT{ messageTypes };

			constexpr [[maybe_unused]] auto LambdaDebugCallback = 
				[](
					vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
					vk::DebugUtilsMessageTypeFlagsEXT type,
					const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
					void* userData
				) static -> vk::Bool32
				{
					if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
						std::println("Validation layer: {}", callbackData->pMessage);
					return false;
				};

			auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT{
				.messageSeverity = severityFlags,
				.messageType = messageTypeFlags,
				.pfnUserCallback = &self.DebugCallback,
				.pUserData = reinterpret_cast<void**>(&self)
			};
			self.debugMessenger = self.instance.createDebugUtilsMessengerEXT(createInfo);
		}
	};
}
