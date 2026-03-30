export module vulkangfx:graphics.corevulkan;
import std;
import :vulkan;
import :glfw;
import :gsl;
import :win32;
import :stlhelpers;
import :error;
import :logging;
import :graphics.vertex;

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
				.CreateImageViews()
				.CreateSyncObjects()
				.CreateGraphicsPipeline()
				.CreateCommandPool()
				.CreateCommandBuffers();
		}

		auto CreateVertexBuffer(this CoreVulkan& self, std::span<const Vertex> vertices) -> Vulkan::BufferHandle
		{
			auto handle = Vulkan::BufferHandle{};

			auto bufferInfo = vkr::VkBufferCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = nullptr,
				.size = sizeof(Vertex) * vertices.size(),
				.usage = vkr::VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				.sharingMode = vkr::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			};
			auto result = Vulkan::Result{
				vkr::vkCreateBuffer(
					self.device->GetHandle(),
					&bufferInfo,
					nullptr,
					&handle.Buffer
				)};
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to create vertex buffer." };

			auto memoryRequirements = vkr::VkMemoryRequirements{};
			vkr::vkGetBufferMemoryRequirements(self.device->GetHandle(), handle.Buffer, &memoryRequirements);
			auto chosenMemoryType = std::uint32_t{
				Vulkan::FindMemoryType(
					self.physicalDevice->GetHandle(),
					memoryRequirements.memoryTypeBits,
					vkr::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | vkr::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
				)};

			auto allocInfo = vkr::VkMemoryAllocateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.pNext = nullptr,
				.allocationSize = memoryRequirements.size,
				.memoryTypeIndex = chosenMemoryType
			};
			vkr::vkAllocateMemory(self.device->GetHandle(), &allocInfo, nullptr, &handle.Memory);

			result = Vulkan::Result{ 
				vkr::vkAllocateMemory(
					self.device->GetHandle(), 
					&allocInfo, 
					nullptr, 
					&handle.Memory
				)};
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to allocate vertex buffer memory." };

			vkr::vkBindBufferMemory(self.device->GetHandle(), handle.Buffer, handle.Memory, 0);

			void* data;
			vkr::vkMapMemory(
				self.device->GetHandle(), 
				handle.Memory, 
				0, 
				bufferInfo.size, 
				0, 
				&data
			);
			std::memcpy(data, vertices.data(), static_cast<std::size_t>(bufferInfo.size));
			vkr::vkUnmapMemory(self.device->GetHandle(), handle.Memory);

			return handle;
		}

		auto DestroyVertexBuffer(this CoreVulkan& self, Vulkan::BufferHandle& handle)
		{
			vkr::vkDestroyBuffer(self.device->GetHandle(), handle.Buffer, nullptr);
			vkr::vkFreeMemory(self.device->GetHandle(), handle.Memory, nullptr);
		}

		void CleanupSwapChain(this CoreVulkan& self)
		{
			self.swapchainImageViews.clear();
			self.swapchain.reset();
		}

		auto WaitForDeviceIdle(this CoreVulkan& self) -> decltype(self)
		{
			auto result = Vulkan::Result{ vkr::vkDeviceWaitIdle(self.device->GetHandle()) };
			if (not result)
				throw Vulkan::VulkanError{ result, "Failed to wait for device idle." };
			return self;
		}

		void RecreateSwapChain(this CoreVulkan& self)
		{
			auto [width, height] = 
				[window = self.window]
				{
					auto dimensions = window->GetFramebufferDimensions();
					while(dimensions.X == 0 or dimensions.Y == 0)
					{
						dimensions = window -> GetFramebufferDimensions();
						glfw::glfwWaitEvents();
					}
					return dimensions;
				}();

			self.device->WaitIdle();
			self.CleanupSwapChain();
			self.CreateSwapChain();
			self.CreateImageViews();
		}

		// This function is tricky. Make sure to read the comments carefully and 
		// understand the synchronization mechanisms in place. The complexity here
		// lies in the fact that some functions consume signals while others 
		// produce them and the synchronisation happens between CPU-GPU and GPU-GPU.
		auto DrawFrame(this CoreVulkan& self, Vulkan::BufferHandle& vertexBuffer) -> decltype(self)
		{
			// [CPU wait] Blocks until stillRenderingFence[frameIndex] is signalled,
			// guaranteeing the previous submit using this frame slot has completed.
			// The fence is created in the signalled state so the first frame doesn't deadlock.
			if (auto result = Vulkan::Result{ self.stillRenderingFences[self.frameIndex].Wait()}; not result)
				throw Vulkan::VulkanError{result, "Failed to wait for fence."};

			// [Signals] imageAvailableSemaphore[frameIndex] (GPU-side, when the
			// presentation engine releases the image). The semaphore is safe to
			// reuse here because the fence above guarantees the previous submit
			// — which consumed it — has completed.
			auto acquiredImage = self.swapchain->AcquireNextImage(self.imageAvailableSemaphores[self.frameIndex].GetHandle());
			if (acquiredImage.Result.IsOutOfDate())
			{
				self.RecreateSwapChain();
				return self;
			}
			if (acquiredImage.Result.Failed() and not acquiredImage.Result.IsSuboptimal())
				throw Vulkan::VulkanError{ acquiredImage.Result, "Failed to acquire swapchain image." };

			self.stillRenderingFences[self.frameIndex].Reset();
			self.commandBuffers[self.frameIndex].Reset();
			self.RecordCommandBuffer(acquiredImage.ImageIndex, vertexBuffer);

			// Submission of the command buffer for execution to the graphics queue.
			// [Consumes] imageAvailableSemaphore[frameIndex] — waits for the acquired
			//            image to be ready before the color attachment output stage.
			// [Signals]  renderFinishedSemaphore[imageIndex] — notifies present that
			//            rendering to this image is complete. Indexed per-image (not
			//            per-frame) because vkAcquireNextImageKHR returning this imageIndex
			//            is the only guarantee that the previous present consuming this
			//            semaphore has finished.
			// [Signals]  stillRenderingFence[frameIndex] — CPU-side notification that
			//            this submit has completed, allowing the frame slot to be reused.
			auto waitSemaphores = std::array{ self.imageAvailableSemaphores[self.frameIndex].GetHandle() };
			auto signalSemaphores = std::array{ self.renderFinishedSemaphores[acquiredImage.ImageIndex].GetHandle() };
			[&waitSemaphores, &signalSemaphores, &self]
			{
				auto destinationStageMask = vkr::VkPipelineStageFlags{ vkr::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
				auto commandBuffers = std::array{ self.commandBuffers[self.frameIndex].GetHandle() };
				auto submitInfo = vkr::VkSubmitInfo{
					.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO,
					.pNext = nullptr,
					.waitSemaphoreCount = static_cast<std::uint32_t>(waitSemaphores.size()),
					.pWaitSemaphores = waitSemaphores.data(),
					.pWaitDstStageMask = &destinationStageMask,
					.commandBufferCount = static_cast<std::uint32_t>(commandBuffers.size()),
					.pCommandBuffers = commandBuffers.data(),
					.signalSemaphoreCount = static_cast<std::uint32_t>(signalSemaphores.size()),
					.pSignalSemaphores = signalSemaphores.data()
				};
				auto submitResult = Vulkan::Result{
					vkr::vkQueueSubmit(
						self.deviceQueue->GetQueue(),
						1,
						&submitInfo,
						self.stillRenderingFences[self.frameIndex].GetHandle()
					) };
				if (not submitResult)
					throw Vulkan::VulkanError{ submitResult, "Failed to submit draw command buffer." };
			}();

			// Presentation can only happen after rendering is finished on the same frame.
			// [Consumes] renderFinishedSemaphore[imageIndex] — waits for rendering
			//            to finish (submitted directly above) before displaying the 
			//			  image. Not covered by the fence; the semaphore is only 
			//			  safe to re-signal when this image is re-acquired.
			auto result = 
				[imageIndex = acquiredImage.ImageIndex, &waitSemaphores, &signalSemaphores, &self]
				{
					auto swapchains = std::array{ self.swapchain->GetHandle() };
					auto presentInfo = vkr::VkPresentInfoKHR{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
						.pNext = nullptr,
						.waitSemaphoreCount = static_cast<std::uint32_t>(signalSemaphores.size()),
						.pWaitSemaphores = signalSemaphores.data(),
						.swapchainCount = static_cast<std::uint32_t>(swapchains.size()),
						.pSwapchains = swapchains.data(),
						.pImageIndices = &imageIndex,
						.pResults = nullptr
					};
					return Vulkan::Result{ vkr::vkQueuePresentKHR(self.deviceQueue->GetQueue(), &presentInfo) };
				}();
			if (result.IsOutOfDate() or result.IsSuboptimal())
				self.RecreateSwapChain();
			else if (result.Failed())
				throw Vulkan::VulkanError{ result, "Failed to present swapchain image." };

			self.frameIndex = (self.frameIndex + 1) % self.MaxFramesInFlight;
			return decltype(self)(self);
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
			auto factory = Vulkan::DeviceFactory{
				.Info = {
					.QueueCreateInfos = {
						{
							.Flags = 0,
							.QueueFamilyIndex = self.selectedQueue.FamilyIndex,
							.QueuePriorities = {1.f}
						}
					},
					.EnabledExtensions = {vkr::Extensions::SwapChain},
					.EnabledFeatures13 = {.synchronization2 = true, .dynamicRendering = true },
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

			self.swapchain = Vulkan::Swapchain{ factory() };
			self.swapchainImages = self.swapchain->GetImages();
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
			// Render finished semaphores need to match the swapchain image count. This is independent of the frames in flight.
			// While frames in flight and swapchain image counts can be made to match, in practise it's common to have max 
			// frames in flight = swapchain image count - 1.
			for (auto i = 0u; i < self.swapchainImages.size(); i++)
				self.renderFinishedSemaphores.emplace_back(Vulkan::Sync::BinarySemaphore::Create(self.device->GetHandle()));

			for (auto i = 0u; i < MaxFramesInFlight; i++)
			{
				self.imageAvailableSemaphores.emplace_back(Vulkan::Sync::BinarySemaphore::Create(self.device->GetHandle()));
				self.stillRenderingFences.emplace_back(Vulkan::Sync::Fence::Create(self.device->GetHandle(), true));
			}
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

			auto bindingDescription = Vertex::GetBindingDescription();
			auto attributeDescriptions = Vertex::GetAttributeDescription();
			auto vertexInputInfo = vkr::VkPipelineVertexInputStateCreateInfo{
				.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.vertexBindingDescriptionCount = 1,
				.pVertexBindingDescriptions = &bindingDescription,
				.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributeDescriptions.size()),
				.pVertexAttributeDescriptions = attributeDescriptions.data()
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

			// Enable alpha blending for transparency.
			auto colorBlendAttachment = vkr::VkPipelineColorBlendAttachmentState{
				.blendEnable = true,
				.srcColorBlendFactor = vkr::VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA,
				.dstColorBlendFactor = vkr::VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
				.colorBlendOp = vkr::VkBlendOp::VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = vkr::VkBlendFactor::VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = vkr::VkBlendFactor::VK_BLEND_FACTOR_ZERO,
				.alphaBlendOp = vkr::VkBlendOp::VK_BLEND_OP_ADD,
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
				Vulkan::CommandPool::Factory{
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
			// Each frame in flight needs its own command buffer to record commands for that frame.
			self.logger.Info("Creating command buffers...");
			for (auto i = 0; i < MaxFramesInFlight; i++)
				self.commandBuffers.emplace_back(self.commandPool->CreatePrimaryCommandBuffer());
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

		auto RecordCommandBuffer(
			this CoreVulkan& self, 
			std::uint32_t imageIndex,
			Vulkan::BufferHandle& vertexBuffer
		) -> decltype(self)
		{
			// Begin command buffer recording
			self.commandBuffers[self.frameIndex]
				.Begin()
				// Transition swapchain image: undefined -> color attachment optimal
				.PipelineBarrier2Ex(
					vkr::VkImageMemoryBarrier2{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.srcStageMask = vkr::PipelineStage2::ColorAttachmentOutput,
						.srcAccessMask = vkr::Access2::None,
						.dstStageMask = vkr::PipelineStage2::ColorAttachmentOutput,
						.dstAccessMask = vkr::Access2::ColorAttachmentWrite,
						.oldLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
						.newLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.srcQueueFamilyIndex = vkr::QueueFamilyIgnored,
						.dstQueueFamilyIndex = vkr::QueueFamilyIgnored,
						.image = self.swapchainImages[imageIndex],
						.subresourceRange = {
							.aspectMask = vkr::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						}
					})
				// Begin dynamic rendering
				.BeginRendering(
					vkr::VkRect2D{
						.offset = { 0, 0 },
						.extent = self.swapChainExtent
					},
					vkr::VkRenderingAttachmentInfo{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
						.imageView = self.swapchainImageViews[imageIndex].GetHandle(),
						.imageLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.loadOp = vkr::VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
						.storeOp = vkr::VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
						.clearValue = vkr::VkClearValue{.color = {.float32 = { 0.0f, 0.0f, 0.0f, 1.0f } } }
					}
				)
				// Bind pipeline and set dynamic state
				.BindPipeline(vkr::VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, self.pipeline->GetHandle())
				.SetViewport(
					vkr::VkViewport{
						.x = 0.f, 
						.y = 0.f,
						.width = static_cast<float>(self.swapChainExtent.width),
						.height = static_cast<float>(self.swapChainExtent.height),
						.minDepth = 0.f, 
						.maxDepth = 1.f
					})
				.SetScissor(
					vkr::VkRect2D{
						.offset = { 0, 0 },
						.extent = self.swapChainExtent
					})
				// Draw the hardcoded triangle (3 vertices defined in the vertex shader)
				.BindAndDrawVertexBuffer(vertexBuffer.Buffer, 3, 1, 0, 0)
				.EndRendering()
				// Transition swapchain image: color attachment optimal -> present src
				.PipelineBarrier2Ex(
					vkr::VkImageMemoryBarrier2{
						.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
						.srcStageMask = vkr::PipelineStage2::ColorAttachmentOutput,
						.srcAccessMask = vkr::Access2::ColorAttachmentWrite,
						.dstStageMask = vkr::PipelineStage2::BottomOfPipe,
						.dstAccessMask = vkr::Access2::None,
						.oldLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.newLayout = vkr::VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
						.srcQueueFamilyIndex = vkr::QueueFamilyIgnored,
						.dstQueueFamilyIndex = vkr::QueueFamilyIgnored,
						.image = self.swapchainImages[imageIndex],
						.subresourceRange = {
							.aspectMask = vkr::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
							.baseMipLevel = 0,
							.levelCount = 1,
							.baseArrayLayer = 0,
							.layerCount = 1
						}
					})
				.End();

			return decltype(self)(self);
		}

		auto Teardown(this CoreVulkan& self) -> decltype(self)
		{
			self.logger.Info("Tearing down Vulkan resources...");
			self.device->WaitIdle();
			self.CleanupSwapChain();
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
		constexpr static auto MaxFramesInFlight = 2u;
		unsigned frameIndex = 0;

		Vulkan::Instance::MainInstance instance;
		glfw::Window* window = nullptr;
		vkr::VkDebugUtilsMessengerEXT debugMessenger = nullptr;
		std::optional<Vulkan::Surface> surface;
		std::optional<Vulkan::PhysicalDevice> physicalDevice;
		std::optional<Vulkan::Device> device;
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
		std::vector<Vulkan::CommandBuffer> commandBuffers;
		std::vector<Vulkan::Sync::BinarySemaphore> imageAvailableSemaphores;
		std::vector<Vulkan::Sync::BinarySemaphore> renderFinishedSemaphores;
		std::vector<Vulkan::Sync::Fence> stillRenderingFences;
	};
}