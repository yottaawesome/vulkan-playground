export module vulkangfx:graphics.corevulkan;
import std;
import :vulkan;
import :glfw;
import :gsl;
import :win32;
import :stlhelpers;
import :error;
import :logging;

export namespace Graphics
{
	class CoreVulkan
	{
	public:
		~CoreVulkan()
		{
			Teardown();
		}

		explicit CoreVulkan(gsl::not_null<glfw::Window*> window)	
			: window(window)
		{ }		

		void Initialise(this CoreVulkan& self)
		{
			self.CreateInstance()
				.AddDebugMessenger()
				.PickPhysicalDevice()
				.CreateSurface()
				.CreateLogicalDevice()
				.CreateSwapChain()
				.CreateRenderPass()
				.CreateImageViews()
				.CreateSyncObjects()
				.CreateGraphicsPipeline()
				.CreateCommandPool()
				.CreateCommandBuffers();
		}

		// Order of initialisation.
	private:
		auto CreateInstance(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating Vulkan instance...");

			auto requiredExtensions = std::vector{ self.GetRequiredExtensions() };
			auto requiredLayers = std::vector{ self.GetRequiredLayers() };

			auto extensionSupport = Vulkan::Instance::EvaluateExtensionSupport(requiredExtensions);
			if (extensionSupport.HasUnsupported())
			{
				auto message = std::format(
					"Not all required Vulkan extensions are supported. Unsupported extensions: {}",
					std::ranges::views::join_with(extensionSupport.Names, ", ") | std::ranges::to<std::string>()
				);
				throw Error::RuntimeError(message);
			}

			auto layerSupport = Vulkan::Instance::EvaluateLayerSupport(requiredLayers);
			if (not layerSupport.empty())
			{
				auto message = std::format(
					"Not all required Vulkan layers are supported. Unsupported layers: {}",
					std::ranges::views::join_with(layerSupport, ", ") | std::ranges::to<std::string>()
				);
				throw Error::RuntimeError(message);
			}

			auto instanceFactory = Vulkan::Instance::Factory{
				.ApplicationInfo = {
					.ApplicationName = "Graphics Programming with Vulkan and C++",
					.ApplicationVersion = vkr::MakeVersion(1, 0, 0),
					.EngineName = "Vulkangeance",
					.EngineVersion = vkr::MakeVersion(1, 0, 0),
					.ApiVersion = static_cast<std::uint32_t>(vkr::Versions::Vulkan14)
				},
				.InstanceInfo = {
					.Flags = 0,
					.EnabledLayerNames = requiredLayers,
					.EnabledExtensionNames = requiredExtensions,
				}
			};
			self.instance = instanceFactory();
			return self;
		}

		auto AddDebugMessenger(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Setting up debug messenger...");

			constexpr auto severity =
				vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			constexpr auto types =
				vkr::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				vkr::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				vkr::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			constexpr auto callback =
				 [](
					vkr::VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
					vkr::VkDebugUtilsMessageTypeFlagsEXT messageTypes [[maybe_unused]],
					const vkr::VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
					void* pUserData [[maybe_unused]]
				) static -> vkr::VkBool32
				{
					static auto logger = Log::Logger<"Validation layer">();
					if (messageSeverity >= vkr::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
						logger.Info("{}", pCallbackData->pMessage);
					return vkr::False;
				};
			self.debugMessenger = self.instance.SetupDebugMessenger(severity, types, &self, callback);
			return self;
		}

		auto PickPhysicalDevice(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Picking physical device...");

			auto deviceList = Vulkan::PhysicalDeviceList::Enumerate(self.instance.GetHandle());

			deviceList = deviceList
				.FilterByQueueSupport(vkr::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, vkr::VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
				.FilterByPhysicalDeviceType(vkr::VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
			if (deviceList.Devices.empty())
					throw Error::RuntimeError("Failed to find a discrete GPU with graphics support.");
			self.physicalDevice = deviceList.Devices.front();

			auto queueFamilyDetails = 
				self.physicalDevice->GetQueueFamilyDetails()
				.filter(
					[](const Vulkan::DeviceQueueDetails& details) 
					{ 
						return details.SupportsOperations(vkr::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, vkr::VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
					});
			if (queueFamilyDetails.empty())
				throw Error::RuntimeError("Selected physical device does not have any queue families that support graphics and transfer queues.");
			self.selectedQueue = queueFamilyDetails.front();

			return self;
		}

		auto CreateSurface(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating window surface...");

			self.surface = Vulkan::Surface{
				self.window->CreateVulkanSurface(self.instance.GetHandle()),
				self.physicalDevice->GetHandle(),
				self.instance.GetHandle()
			};
			return self;
		}

		auto CreateLogicalDevice(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating logical device...");

			if (not self.physicalDevice)
				throw Error::RuntimeError("Physical device must be selected before creating logical device.");

			auto dynamicRendering = vkr::VkPhysicalDeviceDynamicRenderingFeatures{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
				.dynamicRendering = true
			};
			auto factory = Vulkan::LogicalDeviceFactory{
				.Info = {
					.QueueCreateInfos = {
						{
							.Flags = 0,
							.QueueFamilyIndex = self.selectedQueue.FamilyIndex,
							.QueuePriorities = {1.f}
						}
					},
					.EnabledExtensions = {vkr::Extensions::SwapChain},
					.EnabledFeatures13 = {.dynamicRendering = true},
					.EnabledFeatures14 = {.dynamicRenderingLocalRead = true},
				},
				.PhysicalDevice = self.physicalDevice->Handle,
			};
			self.device = factory();

			self.deviceQueue = Vulkan::DeviceQueue(
				self.physicalDevice->Handle,
				self.device->GetHandle(),
				self.selectedQueue.FamilyIndex,
				0
			);

			return self;
		}
		
		auto CreateSwapChain(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating swap chain...");

			auto surfaceCapabilities = vkr::VkSurfaceCapabilitiesKHR{ self.surface->GetSurfaceCapabilities() };
			self.swapChainExtent = self.ChooseSwapExtent(surfaceCapabilities);
			self.swapChainSurfaceFormat = self.ChooseSwapSurfaceFormat(self.surface->GetSurfaceFormats());

			auto imageCount = std::uint32_t{ surfaceCapabilities.minImageCount + 1 };
			if (surfaceCapabilities.maxImageCount > 0 and imageCount > surfaceCapabilities.maxImageCount)
				imageCount = surfaceCapabilities.maxImageCount;

			auto factory = Vulkan::SwapchainFactory{
				.Info = {
					.flags = 0,
					.surface = self.surface->GetHandle(),
					.minImageCount =
						[&surfaceCapabilities] noexcept -> std::uint32_t
						{
							auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
							if (0 < surfaceCapabilities.maxImageCount
								and surfaceCapabilities.maxImageCount < minImageCount
							) minImageCount = surfaceCapabilities.maxImageCount;
							return minImageCount;
						}(),
					.imageFormat = self.swapChainSurfaceFormat.format,
					.imageColorSpace = self.swapChainSurfaceFormat.colorSpace,
					.imageExtent = self.swapChainExtent,
					.imageArrayLayers = 1,
					.imageUsage = vkr::VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
					.imageSharingMode = vkr::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
					.preTransform = surfaceCapabilities.currentTransform,
					.compositeAlpha = vkr::VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
					.presentMode =
						[
							&surfaceCapabilities, 
							availablePresentModes = self.surface->GetSurfacePresentModes()
						] -> vkr::VkPresentModeKHR
						{
							// Only VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available.
							// VK_PRESENT_MODE_FIFO_KHR is more important for mobile devices,
							// where energy usage matters.
							for (const auto& availablePresentMode : availablePresentModes)
								if (availablePresentMode == vkr::VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
									return availablePresentMode;
							return vkr::VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
						}(),
					.clipped = vkr::True,
					.oldSwapchain = nullptr
				},
				.Device = self.device->GetHandle()
			};

			self.swapchain = Vulkan::Swapchain{ factory(), self.device->GetHandle() };
			self.swapchainImages = self.swapchain->GetImages();
			return decltype(self)(self);
		}

		auto CreateRenderPass(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating render pass...");
			return decltype(self)(self);
		}

		auto CreateImageViews(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating image views...");

			self.swapchainImageViews.clear();
			auto factory = Vulkan::ImageViewFactory{
				.Device = self.device->GetHandle(),
				.ViewType = vkr::VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.Format = self.swapChainSurfaceFormat.format,
				.Components {
					.r = vkr::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = vkr::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = vkr::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = vkr::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
				},
				.SubresourceRange {
					.aspectMask = vkr::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			
			for (vkr::VkImage image : self.swapchainImages)
			{
				factory.Image = image;
				self.swapchainImageViews.emplace_back(factory());
			}

			return decltype(self)(self);
		}

		auto CreateSyncObjects(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating synchronization objects...");
			return decltype(self)(self);
		}

		auto CreateGraphicsPipeline(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Describing graphics pipeline...");
			// load shader modules
			auto [vertShader, fragShader] = self.LoadShaderModules();

			auto vertexStageInfo = vkr::VkPipelineShaderStageCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = vkr::VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertShader.GetHandle(),
				.pName = "main"
			};
			auto fragmentStageInfo = vkr::VkPipelineShaderStageCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = vkr::VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragShader.GetHandle(),
				.pName = "main"
			};
			auto stages = std::array{ vertexStageInfo, fragmentStageInfo };

			auto dynamicStatus = std::array{ vkr::VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT, vkr::VkDynamicState::VK_DYNAMIC_STATE_SCISSOR };
			auto dynamicStateInfo = vkr::VkPipelineDynamicStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.dynamicStateCount = static_cast<std::uint32_t>(dynamicStatus.size()),
				.pDynamicStates = dynamicStatus.data()
			};

			auto viewport = vkr::VkViewport{
				.x = 0.f,
				.y = 0.f,
				.width = static_cast<float>(self.swapChainExtent.width),
				.height = static_cast<float>(self.swapChainExtent.height),
				.minDepth = 0.f,
				.maxDepth = 1.f
			};

			auto scissor = vkr::VkRect2D{
				.offset = {0, 0},
				.extent = self.swapChainExtent
			};

			auto viewportInfo = vkr::VkPipelineViewportStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.viewportCount = 1,
				.pViewports = &viewport,
				.scissorCount = 1,
				.pScissors = &scissor
			};

			auto vertexInputInfo = vkr::VkPipelineVertexInputStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount = 0,
				.pVertexBindingDescriptions = nullptr,
			};
			auto inputAssemblyInfo = vkr::VkPipelineInputAssemblyStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.topology = vkr::VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = false
			};

			auto rasterizationStateInfo = vkr::VkPipelineRasterizationStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.depthClampEnable = false,
				.rasterizerDiscardEnable = false,
				.polygonMode = vkr::VkPolygonMode::VK_POLYGON_MODE_FILL,
				.cullMode = vkr::VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
				.frontFace = vkr::VkFrontFace::VK_FRONT_FACE_CLOCKWISE,
				.depthBiasEnable = false,
				.lineWidth = 1.f,
			};

			auto multisamplingInfo = vkr::VkPipelineMultisampleStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.rasterizationSamples = vkr::VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = false,
			};

			auto colorBlendAttachment = vkr::VkPipelineColorBlendAttachmentState{
				.blendEnable = false,
				.colorWriteMask =
					vkr::VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT |
					vkr::VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
					vkr::VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT |
					vkr::VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT,
			};

			auto colorBlendingInfo = vkr::VkPipelineColorBlendStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.logicOpEnable = false,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachment
			};

			auto pipelineLayoutFactory = Vulkan::PipelineLayoutFactory{.Device = self.device->GetHandle()};
			self.pipelineLayout = pipelineLayoutFactory();

			auto pipelineRenderingInfo = vkr::VkPipelineRenderingCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &self.swapChainSurfaceFormat.format
			};

			auto pipelineFactory = Vulkan::PipelineFactory{
				.Device = self.device->GetHandle(),
				.CreateInfo = {
					.pNext = &pipelineRenderingInfo,
					.stageCount = static_cast<std::uint32_t>(stages.size()),
					.pStages = stages.data(),
					.pVertexInputState = &vertexInputInfo,
					.pInputAssemblyState = &inputAssemblyInfo,
					.pViewportState = &viewportInfo,
					.pRasterizationState = &rasterizationStateInfo,
					.pMultisampleState = &multisamplingInfo,
					.pColorBlendState = &colorBlendingInfo,
					.pDynamicState = &dynamicStateInfo,
					.layout = self.pipelineLayout->GetHandle(),
					.renderPass = nullptr, // renderpass-less
					.subpass = 0,
				}
			};
			self.pipeline = pipelineFactory();

			return decltype(self)(self);
		}

		auto CreateCommandPool(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating command pool...");

			self.commandPool = 
				Vulkan::CommandPoolFactory{
					.Device = self.device->GetHandle(),
					.CreateInfo = {
						.flags = vkr::VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
						.queueFamilyIndex = self.selectedQueue.FamilyIndex
					}
				}();

			return decltype(self)(self);
		}

		auto CreateCommandBuffers(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Creating command buffers...");
			self.commandBuffer = Vulkan::CommandBufferFactory{
				.Device = self.device->GetHandle(),
				.CommandPool = self.commandPool->GetHandle()
			}();

			return decltype(self)(self);
		}

	private: // Static initialisation support functions
		static auto GetRequiredExtensions() -> std::vector<const char*>
		{
			auto rawRequiredExtensions = gsl::span<gsl::czstring>{ glfw::GetRequiredVulkanExtensions() };
			auto vector = std::vector<const char*>{ rawRequiredExtensions.begin(), rawRequiredExtensions.end() };
			vector.push_back(vkr::Extensions::EXTDebugUtils);
			return vector;
		}

		static auto GetRequiredLayers() -> std::vector<const char*>
		{
			constexpr auto enableValidationLayers = bool{ true };
			auto layers = std::vector<const char*>{};
			if constexpr (enableValidationLayers)
				layers.push_back(vkr::Layers::KhronosValidationLayerName);
			return layers;
		}

	private:
		auto ChooseSwapSurfaceFormat(
			this const CoreVulkan& self,
			const std::vector<vkr::VkSurfaceFormatKHR>& availableFormats
		) -> vkr::VkSurfaceFormatKHR
		{
			self.logger.Info("Choosing swap surface format...");
			if (availableFormats.empty())
				throw Error::RuntimeError("No available surface formats found.");
			for (const auto& availableFormat : availableFormats)
				if (availableFormat.format == vkr::VkFormat::VK_FORMAT_B8G8R8A8_SRGB
					and availableFormat.colorSpace == vkr::VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
				) return availableFormat;
			return availableFormats[0];
		}

		auto ChooseSwapExtent(
			this CoreVulkan& self,
			const vkr::VkSurfaceCapabilitiesKHR& surfaceCapabilities
		) -> vkr::VkExtent2D
		{
			self.logger.Info("Choosing swap extent...");
			if (surfaceCapabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
				return surfaceCapabilities.currentExtent;
			auto [width, height] = self.window->GetContentAreaDimensions();
			auto actualExtent = vkr::VkExtent2D{
				.width = static_cast<std::uint32_t>(width),
				.height = static_cast<std::uint32_t>(height)
			};
			actualExtent.width = std::clamp(
				actualExtent.width,
				surfaceCapabilities.minImageExtent.width,
				surfaceCapabilities.maxImageExtent.width
			);
			actualExtent.height = std::clamp(
				actualExtent.height,
				surfaceCapabilities.minImageExtent.height,
				surfaceCapabilities.maxImageExtent.height
			);
			return actualExtent;
		}

	private:
		auto FlushCommands(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Flushing commands...");
			return self;
		}

		auto DrawFrame(this CoreVulkan& self) -> decltype(self)
		{
			return self;
		}

		auto Teardown(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Tearing down Vulkan resources...");
			self.instance.DestroyDebugUtilsMessengerEXT(self.debugMessenger);
			return self;
		}

		// Loads the vertex and fragment shader modules from disk.
		auto LoadShaderModules(this CoreVulkan& self) -> std::pair<Vulkan::ShaderModule, Vulkan::ShaderModule>
		{
			auto base = std::filesystem::path{ "shaders" };
			auto vertPath = base / "vert.spv";
			auto fragPath = base / "frag.spv";

			return { 
				Vulkan::ShaderModule{ Vulkan::ShaderModuleFactory{ self.device->GetHandle(), vertPath }() }, 
				Vulkan::ShaderModule{ Vulkan::ShaderModuleFactory{ self.device->GetHandle(), fragPath }() } 
			};
		}

	private:
		Log::Logger<"CoreVulkan"> logger;

		Vulkan::Instance::MainInstance instance;
		glfw::Window* window = nullptr;
		vkr::VkDebugUtilsMessengerEXT debugMessenger = nullptr;
		std::optional<Vulkan::Surface> surface;
		std::optional<Vulkan::PhysicalDevice> physicalDevice;
		std::optional<Vulkan::LogicalDevice> device;
		std::optional<Vulkan::DeviceQueue> deviceQueue;
		Vulkan::DeviceQueueDetails selectedQueue{};
		vkr::VkExtent2D swapChainExtent;
		vkr::VkSurfaceFormatKHR swapChainSurfaceFormat;
		std::optional<Vulkan::Swapchain> swapchain;
		Vulkan::SwapchainImages swapchainImages;
		std::vector<Vulkan::ImageView> swapchainImageViews;
		std::optional<Vulkan::PipelineLayout> pipelineLayout;
		std::optional<Vulkan::Pipeline> pipeline;
		std::optional<Vulkan::CommandPool> commandPool;
		std::optional<Vulkan::CommandBuffer> commandBuffer;
	};
}