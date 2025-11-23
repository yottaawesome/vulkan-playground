export module helloworld:hellotriangleapplication;
import std;
import vulkan;

export namespace Build
{
	constexpr bool IsDebug =
#ifdef _DEBUG
		true;
#else
		false;
#endif
	constexpr bool IsRelease = not IsDebug;
}

//
//
// Various helper functions and structures for the Hello Triangle application
export namespace HelloTriangle
{
	constexpr int MaxFramesInFlight = 2;

	struct Vertex 
	{
		glm::vec2 pos;
		glm::vec3 color;
		
		static auto GetBindingDescription() -> Vulkan::VkVertexInputBindingDescription
		{
			Vulkan::VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = Vulkan::VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static auto GetAttributeDescriptions() -> std::array<Vulkan::VkVertexInputAttributeDescription, 2>
		{
			std::array<Vulkan::VkVertexInputAttributeDescription, 2> attributeDescriptions{};
			
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = Vulkan::VkFormat::VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = 
				((size_t)&reinterpret_cast<char const volatile&>((((Vertex*)0)->pos)));
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset =
				((size_t)&reinterpret_cast<char const volatile&>((((Vertex*)0)->color)));

			return attributeDescriptions;
		}
	};

	struct QueueFamilyIndices
	{
		std::optional<std::uint32_t> GraphicsFamily;
		std::optional<std::uint32_t> PresentFamily;
		auto IsComplete(this const auto& self) noexcept -> bool 
		{ 
			return self.GraphicsFamily.has_value()
				and self.PresentFamily.has_value(); 
		}
	};

	// Extension -- needs to be loaded manually
	auto CreateDebugUtilsMessengerEXT(
		Vulkan::VkInstance instance,
		const Vulkan::VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const Vulkan::VkAllocationCallbacks* pAllocator,
		Vulkan::VkDebugUtilsMessengerEXT* pDebugMessenger
	) -> Vulkan::VkResult
	{
		auto fn = reinterpret_cast<Vulkan::PFN_vkCreateDebugUtilsMessengerEXT>(
			Vulkan::vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
		);
		return fn
			? fn(instance, pCreateInfo, pAllocator, pDebugMessenger)
			: Vulkan::VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	// Extension -- needs to be loaded manually
	void DestroyDebugUtilsMessengerEXT(
		Vulkan::VkInstance instance,
		Vulkan::VkDebugUtilsMessengerEXT debugMessenger,
		const Vulkan::VkAllocationCallbacks* pAllocator
	)
	{
		auto fn = reinterpret_cast<Vulkan::PFN_vkDestroyDebugUtilsMessengerEXT>(
			Vulkan::vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
		);
		fn ? fn(instance, debugMessenger, pAllocator) : void();
	}

	struct SwapChainSupportDetails
	{
		Vulkan::VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<Vulkan::VkSurfaceFormatKHR> Formats;
		std::vector<Vulkan::VkPresentModeKHR> PresentModes;
	};

	auto ReadFile(const std::filesystem::path& filename) -> std::vector<std::byte>
	{
		std::ifstream file(filename.string(), std::ios_base::ate | std::ios_base::binary);
		if (not file.is_open())
			throw std::runtime_error("Failed to open file: " + filename.string());

		size_t fileSize = file.tellg();
		std::vector<std::byte> buffer(fileSize);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}
}

//
//
// Main application class for the Hello Triangle application
export namespace HelloTriangle
{
	struct Application
	{
		constexpr static int Width = 800;
		constexpr static int Height = 600;
		constexpr static bool EnableValidationLayers = Build::IsDebug;
		constexpr static std::array ValidationLayers{
			"VK_LAYER_KHRONOS_validation"
		};
		constexpr static std::array DeviceExtensions{
			Vulkan::VkHkrSwapchainExtensionName
		};

		void Run(this auto& self)
		{
			self.InitWindow();
			self.InitVulkan();
			self.EnumerateExtensions();
			self.CheckValidationLayerSupport();
			self.MainLoop();
			self.Cleanup();
		}

	private:
		GLFW::GLFWwindow* m_window = nullptr;
		Vulkan::VkInstance m_instance;
		Vulkan::VkDebugUtilsMessengerEXT m_debugMessenger;
		// Destroyed automatically when the m_instance is destroyed.
		Vulkan::VkPhysicalDevice physicalDevice = nullptr;
		Vulkan::VkDevice device;
		// Implicitly destroyed when the device is destroyed.
		Vulkan::VkQueue graphicsQueue;
		Vulkan::VkSurfaceKHR surface;
		Vulkan::VkQueue presentQueue;
		Vulkan::VkSwapchainKHR swapChain;
		std::vector<Vulkan::VkImage> swapChainImages;
		Vulkan::VkFormat swapChainImageFormat;
		Vulkan::VkExtent2D swapChainExtent;
		std::vector<Vulkan::VkImageView> swapChainImageViews;
		Vulkan::VkRenderPass renderPass;
		Vulkan::VkPipelineLayout pipelineLayout;
		Vulkan::VkPipeline graphicsPipeline;
		std::vector<Vulkan::VkFramebuffer> swapChainFramebuffers;
		Vulkan::VkCommandPool commandPool;
		std::vector<Vulkan::VkCommandBuffer> commandBuffers;
		std::vector<Vulkan::VkSemaphore> imageAvailableSemaphores;
		std::vector<Vulkan::VkSemaphore> renderFinishedSemaphores;
		std::vector<Vulkan::VkFence> inFlightFences;
		bool framebufferResized = false;
		std::uint32_t currentFrame = 0;
		const std::vector<Vertex> vertices{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};
		const std::vector<std::uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};
		Vulkan::VkBuffer vertexBuffer;
		Vulkan::VkDeviceMemory vertexBufferMemory;
		Vulkan::VkBuffer indexBuffer;
		Vulkan::VkDeviceMemory indexBufferMemory;

		auto CheckDeviceExtensionSupport(
			this const auto& self, 
			Vulkan::VkPhysicalDevice device
		) noexcept -> bool
		{
			std::uint32_t extensionCount = 0;
			Vulkan::vkEnumerateDeviceExtensionProperties(
				device,
				nullptr,
				&extensionCount,
				nullptr
			);
			std::vector<Vulkan::VkExtensionProperties> availableExtensions(extensionCount);
			Vulkan::vkEnumerateDeviceExtensionProperties(
				device,
				nullptr,
				&extensionCount,
				availableExtensions.data()
			);

			std::set<std::string> requiredExtensions{
				DeviceExtensions.begin(), 
				DeviceExtensions.end()
			};
			for (const auto& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			return requiredExtensions.empty();
		}

		auto CheckValidationLayerSupport(this const auto& self) -> bool
		{
			std::uint32_t layerCount = 0;
			auto result = Vulkan::vkEnumerateInstanceLayerProperties(
				&layerCount,
				nullptr
			);
			if (VkResult::VK_SUCCESS != result)
				throw std::runtime_error("Failed to enumerate instance layer properties count.");

			std::vector<Vulkan::VkLayerProperties> availableLayers(layerCount);
			result = Vulkan::vkEnumerateInstanceLayerProperties(
				&layerCount,
				availableLayers.data()
			);
			if (VkResult::VK_SUCCESS != result)
				throw std::runtime_error("Failed to enumerate instance layer properties count.");

			return std::ranges::all_of(
				ValidationLayers,
				[&availableLayers](std::string_view layerName)
				{
					bool found =
						std::any_of(
							availableLayers.begin(),
							availableLayers.end(),
							[layerName](const Vulkan::VkLayerProperties& layer)
							{
								return layerName == layer.layerName;
							}
						);
					return found
						? true
						: (std::println("Validation layer not found: {}", layerName), false);
				}
			);
		}
		
		auto ChooseSwapExtent(
			this const auto& self,
			const Vulkan::VkSurfaceCapabilitiesKHR& capabilities
		) -> Vulkan::VkExtent2D
		{
			if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
				return capabilities.currentExtent;

			int width; 
			int height;
			GLFW::glfwGetFramebufferSize(self.m_window, &width, &height);
			Vulkan::VkExtent2D actualExtent{
				.width = static_cast<std::uint32_t>(width),
				.height = static_cast<std::uint32_t>(height)
			};
			actualExtent.width = 
				std::clamp(
					actualExtent.width,
					capabilities.minImageExtent.width,
					capabilities.maxImageExtent.width
				);
			actualExtent.height =
				std::clamp(
					actualExtent.height,
					capabilities.minImageExtent.height,
					capabilities.maxImageExtent.height
				);
			return actualExtent;
		}

		auto ChooseSwapPresentMode(
			this const auto& self,
			const std::vector<Vulkan::VkPresentModeKHR>& availablePresentModes
		) -> Vulkan::VkPresentModeKHR
		{
			for (auto&& mode : availablePresentModes)
				if (mode == Vulkan::VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
					return mode;
			return Vulkan::VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		}

		auto ChooseSwapSurfaceFormat(
			this const auto& self,
			const std::vector<Vulkan::VkSurfaceFormatKHR>& availableFormats
		) -> Vulkan::VkSurfaceFormatKHR
		{
			for (const auto& availableFormat : availableFormats)
				if (availableFormat.format == Vulkan::VkFormat::VK_FORMAT_B8G8R8A8_SRGB
					and availableFormat.colorSpace == Vulkan::VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
				) return availableFormat;
			return availableFormats[0];
		}

		void Cleanup(this auto& self)
		{
			self.CleanupSwapChain();

			Vulkan::vkDestroyBuffer(self.device, self.indexBuffer, nullptr);
			Vulkan::vkFreeMemory(self.device, self.indexBufferMemory, nullptr);

			Vulkan::vkDestroyBuffer(self.device, self.vertexBuffer, nullptr);
			Vulkan::vkFreeMemory(self.device, self.vertexBufferMemory, nullptr);

			Vulkan::vkDestroyPipeline(self.device, self.graphicsPipeline, nullptr);
			Vulkan::vkDestroyPipelineLayout(self.device, self.pipelineLayout, nullptr);
			Vulkan::vkDestroyRenderPass(self.device, self.renderPass, nullptr);

			for (size_t i = 0; i < MaxFramesInFlight; i++)
			{
				Vulkan::vkDestroySemaphore(self.device, self.imageAvailableSemaphores[i], nullptr);
				Vulkan::vkDestroyFence(self.device, self.inFlightFences[i], nullptr);
			}
			for (auto&& renderFinishedSemaphore : self.renderFinishedSemaphores)
				Vulkan::vkDestroySemaphore(self.device, renderFinishedSemaphore, nullptr);

			Vulkan::vkDestroyCommandPool(self.device, self.commandPool, nullptr);
			
			Vulkan::vkDestroyDevice(self.device, nullptr);

			if (EnableValidationLayers)
				DestroyDebugUtilsMessengerEXT(self.m_instance, self.m_debugMessenger, nullptr);
			
			Vulkan::vkDestroySurfaceKHR(self.m_instance, self.surface, nullptr);
			Vulkan::vkDestroyInstance(self.m_instance, nullptr);
			
			GLFW::glfwDestroyWindow(self.m_window);
			GLFW::glfwTerminate();
		}

		void CleanupSwapChain(this auto& self)
		{
			for (auto framebuffer : self.swapChainFramebuffers)
				Vulkan::vkDestroyFramebuffer(self.device, framebuffer, nullptr);
			for (auto imageView : self.swapChainImageViews)
				Vulkan::vkDestroyImageView(self.device, imageView, nullptr);
			Vulkan::vkDestroySwapchainKHR(self.device, self.swapChain, nullptr);
		}

		void CopyBuffer(
			this auto& self, 
			Vulkan::VkBuffer srcBuffer, 
			Vulkan::VkBuffer dstBuffer, 
			Vulkan::VkDeviceSize size
		)
		{
			Vulkan::VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = Vulkan::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = self.commandPool;
			allocInfo.commandBufferCount = 1;

			Vulkan::VkCommandBuffer commandBuffer;
			Vulkan::vkAllocateCommandBuffers(self.device, &allocInfo, &commandBuffer);

			Vulkan::VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = Vulkan::VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			Vulkan::vkBeginCommandBuffer(commandBuffer, &beginInfo);

			Vulkan::VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0; // Optional
			copyRegion.dstOffset = 0; // Optional
			copyRegion.size = size;
			Vulkan::vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			Vulkan::vkEndCommandBuffer(commandBuffer);

			Vulkan::VkSubmitInfo submitInfo{};
			submitInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			Vulkan::vkQueueSubmit(self.graphicsQueue, 1, &submitInfo, nullptr);
			Vulkan::vkQueueWaitIdle(self.graphicsQueue);

			Vulkan::vkFreeCommandBuffers(self.device, self.commandPool, 1, &commandBuffer);
		}

		void CreateBuffer(
			this auto& self, 
			Vulkan::VkDeviceSize size, 
			Vulkan::VkBufferUsageFlags usage,
			Vulkan::VkMemoryPropertyFlags properties,
			Vulkan::VkBuffer& buffer, 
			Vulkan::VkDeviceMemory& bufferMemory
		)
		{
			Vulkan::VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = sizeof(self.vertices[0]) * self.vertices.size();
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = Vulkan::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

			if (Vulkan::vkCreateBuffer(self.device, &bufferInfo, nullptr, &buffer) != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create buffer.");

			Vulkan::VkMemoryRequirements memRequirements;
			Vulkan::vkGetBufferMemoryRequirements(self.device, buffer, &memRequirements);

			Vulkan::VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = self.FindMemoryType(memRequirements.memoryTypeBits, properties);
			if (Vulkan::vkAllocateMemory(self.device, &allocInfo, nullptr, &bufferMemory) != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to allocate vertex buffer memory.");

			Vulkan::vkBindBufferMemory(self.device, buffer, bufferMemory, 0);
		}

		void CreateCommandBuffers(this auto& self)
		{
			self.commandBuffers.resize(MaxFramesInFlight);

			Vulkan::VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = self.commandPool;
			allocInfo.level = Vulkan::VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = (uint32_t)self.commandBuffers.size();;

			auto result = Vulkan::vkAllocateCommandBuffers(
				self.device, 
				&allocInfo, 
				self.commandBuffers.data()
			);
			if (result != Vulkan::VkResult::VK_SUCCESS) 
				throw std::runtime_error("Failed to allocate command buffers.");
		}

		void CreateCommandPool(this auto& self) 
		{
			QueueFamilyIndices queueFamilyIndices = self.FindQueueFamilies(self.physicalDevice);

			Vulkan::VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = Vulkan::VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

			auto result = Vulkan::vkCreateCommandPool(
				self.device, 
				&poolInfo, 
				nullptr, 
				&self.commandPool
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create command pool.");
		}

		void CreateFramebuffers(this auto& self)
		{
			self.swapChainFramebuffers.resize(self.swapChainImageViews.size());
			for (size_t i = 0; i < self.swapChainImageViews.size(); i++) {
				Vulkan::VkImageView attachments[]{ self.swapChainImageViews[i] };

				Vulkan::VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = self.renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = self.swapChainExtent.width;
				framebufferInfo.height = self.swapChainExtent.height;
				framebufferInfo.layers = 1;

				auto result = Vulkan::vkCreateFramebuffer(
					self.device, 
					&framebufferInfo, 
					nullptr, 
					&self.swapChainFramebuffers[i]
				);
				if (result != Vulkan::VkResult::VK_SUCCESS) 
					throw std::runtime_error("Failed to create framebuffer.");
			}
		}

		void CreateGraphicsPipeline(this auto& self)
		{
			auto vertShaderCode = ReadFile("shaders/vert.spv");
			auto fragShaderCode = ReadFile("shaders/frag.spv");

			Vulkan::VkShaderModule vertShaderModule = self.CreateShaderModule(vertShaderCode);
			Vulkan::VkShaderModule fragShaderModule = self.CreateShaderModule(fragShaderCode);

			Vulkan::VkPipelineShaderStageCreateInfo vertShaderStageInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = Vulkan::VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertShaderModule,
				.pName = "main" // can have multiple entry points that change the behavior of the shader
			};

			Vulkan::VkPipelineShaderStageCreateInfo fragShaderStageInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage = Vulkan::VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragShaderModule,
				.pName = "main"
			};

			Vulkan::VkPipelineShaderStageCreateInfo shaderStages[]{ 
				vertShaderStageInfo, 
				fragShaderStageInfo 
			};

			Vulkan::VkPipelineLayoutCreateInfo pipelineLayoutInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = 0, // Optional
				.pSetLayouts = nullptr, // Optional
				.pushConstantRangeCount = 0, // Optional
				.pPushConstantRanges = nullptr // Optional
			};

			auto result = Vulkan::vkCreatePipelineLayout(
				self.device, 
				&pipelineLayoutInfo, 
				nullptr, 
				&self.pipelineLayout
			);
			if(result != VK_SUCCESS)
				throw std::runtime_error("Failed to create pipeline layout.");

			// ADD HERE
			auto bindingDescription = Vertex::GetBindingDescription();
			auto attributeDescriptions = Vertex::GetAttributeDescriptions();

			Vulkan::VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			vertexInputInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			Vulkan::VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			inputAssembly.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = Vulkan::VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = false;

			Vulkan::VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)self.swapChainExtent.width;
			viewport.height = (float)self.swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			Vulkan::VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = self.swapChainExtent;

			std::vector<Vulkan::VkDynamicState> dynamicStates = {
				Vulkan::VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT,
				Vulkan::VkDynamicState::VK_DYNAMIC_STATE_SCISSOR
			};

			Vulkan::VkPipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			Vulkan::VkPipelineViewportStateCreateInfo viewportState{};
			viewportState.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			Vulkan::VkPipelineRasterizationStateCreateInfo rasterizer{};
			rasterizer.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = false;
			rasterizer.rasterizerDiscardEnable = false;
			rasterizer.polygonMode = Vulkan::VkPolygonMode::VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = Vulkan::VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = Vulkan::VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = false;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

			Vulkan::VkPipelineMultisampleStateCreateInfo multisampling{};
			multisampling.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = false;
			multisampling.rasterizationSamples = Vulkan::VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = false; // Optional
			multisampling.alphaToOneEnable = false; // Optional

			// See for alpha blending: https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
			Vulkan::VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = Vulkan::VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | Vulkan::VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT | Vulkan::VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | Vulkan::VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = false;
			colorBlendAttachment.srcColorBlendFactor = Vulkan::VkBlendFactor::VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstColorBlendFactor = Vulkan::VkBlendFactor::VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.colorBlendOp = Vulkan::VkBlendOp::VK_BLEND_OP_ADD; // Optional
			colorBlendAttachment.srcAlphaBlendFactor = Vulkan::VkBlendFactor::VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstAlphaBlendFactor = Vulkan::VkBlendFactor::VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.alphaBlendOp = Vulkan::VkBlendOp::VK_BLEND_OP_ADD; // Optional

			Vulkan::VkPipelineColorBlendStateCreateInfo colorBlending{};
			colorBlending.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = false;
			colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f; // Optional
			colorBlending.blendConstants[1] = 0.0f; // Optional
			colorBlending.blendConstants[2] = 0.0f; // Optional
			colorBlending.blendConstants[3] = 0.0f; // Optional
			// ADD HERE

			Vulkan::VkGraphicsPipelineCreateInfo pipelineInfo{};
			pipelineInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = nullptr; // Optional
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.pDynamicState = &dynamicState;
			pipelineInfo.layout = self.pipelineLayout;
			pipelineInfo.renderPass = self.renderPass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = nullptr; // Optional
			pipelineInfo.basePipelineIndex = -1; // Optional

			result = Vulkan::vkCreateGraphicsPipelines(
				self.device,
				nullptr, 
				1, 
				&pipelineInfo, 
				nullptr, 
				&self.graphicsPipeline
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create graphics pipeline.");

			Vulkan::vkDestroyShaderModule(self.device, fragShaderModule, nullptr);
			Vulkan::vkDestroyShaderModule(self.device, vertShaderModule, nullptr);
		}

		void CreateImageViews(this auto& self)
		{
			self.swapChainImageViews.resize(self.swapChainImages.size());
			for (size_t i = 0; i < self.swapChainImages.size(); i++)
			{
				Vulkan::VkImageViewCreateInfo createInfo{
					.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.image = self.swapChainImages[i],
					.viewType = Vulkan::VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
					.format = self.swapChainImageFormat,
					.components{
						.r = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
						.g = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
						.b = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
						.a = Vulkan::VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY
					},
					.subresourceRange{
						.aspectMask = Vulkan::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
						.baseMipLevel = 0,
						.levelCount = 1,
						.baseArrayLayer = 0,
						.layerCount = 1
					}
				};
				auto result = Vulkan::vkCreateImageView(
					self.device,
					&createInfo,
					nullptr,
					&self.swapChainImageViews[i]
				);
				if (result != Vulkan::VkResult::VK_SUCCESS)
					throw std::runtime_error("Failed to create image views.");
			}
		}

		void CreateIndexBuffer(this auto& self)
		{
			Vulkan::VkDeviceSize bufferSize = sizeof(self.indices[0]) * self.indices.size();

			Vulkan::VkBuffer stagingBuffer;
			Vulkan::VkDeviceMemory stagingBufferMemory;
			self.CreateBuffer(
				bufferSize, 
				Vulkan::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, 
				stagingBufferMemory
			);

			void* data;
			Vulkan::vkMapMemory(self.device, stagingBufferMemory, 0, bufferSize, 0, &data);
			std::memcpy(data, self.indices.data(), (size_t)bufferSize);
			Vulkan::vkUnmapMemory(self.device, stagingBufferMemory);

			self.CreateBuffer(
				bufferSize, 
				Vulkan::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | Vulkan::VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				self.indexBuffer,
				self.indexBufferMemory
			);

			self.CopyBuffer(stagingBuffer, self.indexBuffer, bufferSize);

			Vulkan::vkDestroyBuffer(self.device, stagingBuffer, nullptr);
			Vulkan::vkFreeMemory(self.device, stagingBufferMemory, nullptr);
		}

		void CreateInstance(this auto& self)
		{
			if (EnableValidationLayers and not self.CheckValidationLayerSupport())
				throw std::runtime_error("Validation layers requested, but not available");

			Vulkan::VkApplicationInfo appInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = "Hello Triangle",
				.applicationVersion = Vulkan::MakeVersion(1, 0, 0),
				.pEngineName = "No Engine",
				.engineVersion = Vulkan::MakeVersion(1, 0, 0),
				.apiVersion = Vulkan::ApiVersion1
			};

			auto extensions = self.GetRequiredExtensions();

			Vulkan::VkInstanceCreateInfo createInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pApplicationInfo = &appInfo,
				.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
				.ppEnabledExtensionNames = extensions.data(),
			};

			Vulkan::VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			if (EnableValidationLayers)
			{
				self.PopulateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
				createInfo.ppEnabledLayerNames = ValidationLayers.data();
				createInfo.pNext =
					reinterpret_cast<Vulkan::VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
			}
			Vulkan::VkResult result =
				Vulkan::vkCreateInstance(&createInfo, nullptr, &self.m_instance);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create instance!");
		}

		void CreateLogicalDevice(this auto& self)
		{
			QueueFamilyIndices indices = self.FindQueueFamilies(self.physicalDevice);

			std::vector<Vulkan::VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<std::uint32_t> uniqueQueueFamilies{ 
				indices.GraphicsFamily.value(), 
				indices.PresentFamily.value() 
			};
			float queuePriority = 1.0f;
			for (std::uint32_t queueFamily : uniqueQueueFamilies) 
			{
				Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = indices.GraphicsFamily.value(),
				.queueCount = 1
			};
			queueCreateInfo.pQueuePriorities = &queuePriority;

			Vulkan::VkPhysicalDeviceFeatures deviceFeatures{};
			Vulkan::VkDeviceCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
				.pQueueCreateInfos = queueCreateInfos.data(),
				.enabledExtensionCount = static_cast<std::uint32_t>(DeviceExtensions.size()),
				.ppEnabledExtensionNames = DeviceExtensions.data(),
				.pEnabledFeatures = &deviceFeatures,
			};
			if (EnableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<std::uint32_t>(ValidationLayers.size());
				createInfo.ppEnabledLayerNames = ValidationLayers.data();
			}

			auto result = Vulkan::vkCreateDevice(
				self.physicalDevice,
				&createInfo,
				nullptr,
				&self.device
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create logical device.");

			Vulkan::vkGetDeviceQueue(
				self.device,
				indices.GraphicsFamily.value(),
				0,
				&self.graphicsQueue
			);
			Vulkan::vkGetDeviceQueue(
				self.device, 
				indices.PresentFamily.value(), 
				0, 
				&self.presentQueue
			);
		}

		auto CreateShaderModule(
			this auto& self, 
			const std::vector<std::byte>& code
		) -> Vulkan::VkShaderModule
		{
			Vulkan::VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			Vulkan::VkShaderModule shaderModule;
			auto result = Vulkan::vkCreateShaderModule(self.device, &createInfo, nullptr, &shaderModule);
			if (result != Vulkan::VkResult::VK_SUCCESS) 
				throw std::runtime_error("Failed to create shader module.");

			return shaderModule;
		}

		void CreateRenderPass(this auto& self)
		{
			Vulkan::VkAttachmentDescription colorAttachment{};
			colorAttachment.format = self.swapChainImageFormat;
			colorAttachment.samples = Vulkan::VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = Vulkan::VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = Vulkan::VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = Vulkan::VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = Vulkan::VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = Vulkan::VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = Vulkan::VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			Vulkan::VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = Vulkan::VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			Vulkan::VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = Vulkan::VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			Vulkan::VkSubpassDependency dependency{};
			dependency.srcSubpass = Vulkan::VkSubpassExternalDependency;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = Vulkan::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = Vulkan::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = Vulkan::VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			Vulkan::VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			auto result = Vulkan::vkCreateRenderPass(self.device, &renderPassInfo, nullptr, &self.renderPass);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create render pass.");
		}

		void CreateSurface(this auto& self)
		{
			Vulkan::VkWin32SurfaceCreateInfoKHR createInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				.hinstance = Win32::GetModuleHandleW(nullptr),
				.hwnd = GLFW::glfwGetWin32Window(self.m_window),
			};

			auto result = Vulkan::vkCreateWin32SurfaceKHR(
				self.m_instance,
				&createInfo,
				nullptr,
				&self.surface
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create window surface.");
		}

		void CreateSwapChain(this auto& self)
		{
			SwapChainSupportDetails swapChainSupport = self.QuerySwapChainSupport(self.physicalDevice);

			Vulkan::VkSurfaceFormatKHR surfaceFormat =
				self.ChooseSwapSurfaceFormat(swapChainSupport.Formats);
			Vulkan::VkPresentModeKHR presentMode =
				self.ChooseSwapPresentMode(swapChainSupport.PresentModes);
			Vulkan::VkExtent2D extent = self.ChooseSwapExtent(swapChainSupport.Capabilities);

			std::uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
			if (
				swapChainSupport.Capabilities.maxImageCount > 0
				and imageCount > swapChainSupport.Capabilities.maxImageCount
			) imageCount = swapChainSupport.Capabilities.maxImageCount;

			Vulkan::VkSwapchainCreateInfoKHR createInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface = self.surface,
				.minImageCount = imageCount,
				.imageFormat = surfaceFormat.format,
				.imageColorSpace = surfaceFormat.colorSpace,
				.imageExtent = extent,
				.imageArrayLayers = 1,
				.imageUsage = Vulkan::VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			};

			QueueFamilyIndices indices = self.FindQueueFamilies(self.physicalDevice);
			std::uint32_t queueFamilyIndices[]{
				indices.GraphicsFamily.value(),
				indices.PresentFamily.value()
			};

			if (indices.GraphicsFamily != indices.PresentFamily)
			{
				createInfo.imageSharingMode = Vulkan::VkSharingMode::VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else
			{
				createInfo.imageSharingMode = Vulkan::VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;
				createInfo.pQueueFamilyIndices = nullptr;
			}
			createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
			createInfo.compositeAlpha = Vulkan::VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = true;
			createInfo.oldSwapchain = nullptr;

			auto result = Vulkan::vkCreateSwapchainKHR(
				self.device,
				&createInfo,
				nullptr,
				&self.swapChain
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to create swap chain.");

			std::uint32_t actualImageCount;
			Vulkan::vkGetSwapchainImagesKHR(
				self.device, 
				self.swapChain, 
				&actualImageCount,
				nullptr
			);
			self.swapChainImages.resize(actualImageCount);
			Vulkan::vkGetSwapchainImagesKHR(
				self.device, 
				self.swapChain, 
				&actualImageCount,
				self.swapChainImages.data()
			);

			self.swapChainImageFormat = surfaceFormat.format;
			self.swapChainExtent = extent;
		}

		void CreateSyncObjects(this auto& self)
		{
			self.imageAvailableSemaphores.resize(MaxFramesInFlight);
			self.renderFinishedSemaphores.resize(self.swapChainImages.size());
			self.inFlightFences.resize(MaxFramesInFlight);

			Vulkan::VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			Vulkan::VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = Vulkan::VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;

			for (int i = 0; i < MaxFramesInFlight; i++)
			{
				if (
					Vulkan::vkCreateSemaphore(self.device, &semaphoreInfo, nullptr, &self.imageAvailableSemaphores[i]) != VK_SUCCESS
					or Vulkan::vkCreateFence(self.device, &fenceInfo, nullptr, &self.inFlightFences[i]) != VK_SUCCESS
				) throw std::runtime_error("failed to create synchronization objects for a frame!");
			}

			// The tutorial triggers a validation error and so followed the advice in this comment
			// to resolve it: http://disq.us/p/32wdmkz
			for (auto& renderFinishedSemaphore : self.renderFinishedSemaphores)
				if (Vulkan::vkCreateSemaphore(self.device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
					throw std::runtime_error("failed to create synchronization objects for a frame!");
		}

		void CreateVertexBuffer(this auto& self) 
		{
			Vulkan::VkDeviceSize bufferSize = sizeof(self.vertices[0]) * self.vertices.size();

			Vulkan::VkBuffer stagingBuffer;
			Vulkan::VkDeviceMemory stagingBufferMemory;
			self.CreateBuffer(
				bufferSize, 
				Vulkan::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer,
				stagingBufferMemory
			);

			void* data;
			Vulkan::vkMapMemory(self.device, stagingBufferMemory, 0, bufferSize, 0, &data);
			std::memcpy(data, self.vertices.data(), (size_t)bufferSize);
			Vulkan::vkUnmapMemory(self.device, stagingBufferMemory);

			self.CreateBuffer(
				bufferSize, 
				Vulkan::VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | Vulkan::VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				self.vertexBuffer, 
				self.vertexBufferMemory
			);

			self.CopyBuffer(stagingBuffer, self.vertexBuffer, bufferSize);
			self.CopyBuffer(stagingBuffer, self.vertexBuffer, bufferSize);

			Vulkan::vkDestroyBuffer(self.device, stagingBuffer, nullptr);
			Vulkan::vkFreeMemory(self.device, stagingBufferMemory, nullptr);
		}

		// stdcall on Windows, but this is ignored on x64.
		static auto DebugCallback(
			Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			Vulkan::VkDebugUtilsMessageTypeFlagsEXT messageType,
			const Vulkan::VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		) -> Vulkan::VkBool32
		{
			if (messageSeverity >= Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				std::println("Validation layer: {}", pCallbackData->pMessage);
			}

			return false;
		}

		void DrawFrame(this auto& self)
		{
			Vulkan::vkWaitForFences(
				self.device, 
				1, 
				&self.inFlightFences[self.currentFrame],
				true, 
				std::numeric_limits<std::uint64_t>::max()
			);

			std::uint32_t imageIndex;
			auto result = Vulkan::vkAcquireNextImageKHR(
				self.device,
				self.swapChain,
				std::numeric_limits<std::uint64_t>::max(),
				self.imageAvailableSemaphores[self.currentFrame],
				nullptr,
				&imageIndex
			);
			if (result == Vulkan::VkResult::VK_ERROR_OUT_OF_DATE_KHR)
			{
				self.RecreateSwapChain();
				return;
			}
			if (result != Vulkan::VkResult::VK_SUCCESS and result != Vulkan::VkResult::VK_SUBOPTIMAL_KHR)
				throw std::runtime_error("Failed to acquire swap chain image.");

			Vulkan::vkResetFences(self.device, 1, &self.inFlightFences[self.currentFrame]);

			Vulkan::vkResetCommandBuffer(self.commandBuffers[self.currentFrame], 0);
			self.RecordCommandBuffer(self.commandBuffers[self.currentFrame], imageIndex);

			Vulkan::VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			Vulkan::VkSemaphore waitSemaphores[]{ self.imageAvailableSemaphores[self.currentFrame] };
			Vulkan::VkPipelineStageFlags waitStages[]{
				Vulkan::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			};
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &self.commandBuffers[self.currentFrame];

			Vulkan::VkSemaphore signalSemaphores[]{ self.renderFinishedSemaphores[imageIndex] };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = signalSemaphores;

			result = Vulkan::vkQueueSubmit(
				self.graphicsQueue, 
				1, 
				&submitInfo, 
				self.inFlightFences[self.currentFrame]
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to submit draw command buffer.");

			Vulkan::VkPresentInfoKHR presentInfo{};
			presentInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;

			Vulkan::VkSwapchainKHR swapChains[] = { self.swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;

			presentInfo.pResults = nullptr; // Optional

			result = Vulkan::vkQueuePresentKHR(self.presentQueue, &presentInfo);
			if (result == VK_ERROR_OUT_OF_DATE_KHR or result == VK_SUBOPTIMAL_KHR or self.framebufferResized)
			{
				self.framebufferResized = false;
				self.RecreateSwapChain();
			}
			else if (result != VK_SUCCESS) 
				throw std::runtime_error("failed to present swap chain image!");

			self.currentFrame = (self.currentFrame + 1) % MaxFramesInFlight;
		}

		void EnumerateExtensions(this const auto& self)
		{
			std::uint32_t extensionsCount = 0;
			auto result =
				Vulkan::vkEnumerateInstanceExtensionProperties(
					nullptr,
					&extensionsCount,
					nullptr
				);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to enumerate extensions count.");

			std::vector<Vulkan::VkExtensionProperties> extensions(extensionsCount);
			result = Vulkan::vkEnumerateInstanceExtensionProperties(
				nullptr,
				&extensionsCount,
				extensions.data()
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to enumerate extensions.");

			for (const auto& extension : extensions)
				std::println("Extension: {}", extension.extensionName);
		}

		auto FindMemoryType(this const auto& self, std::uint32_t typeFilter, std::uint32_t properties) -> std::uint32_t
		{
			Vulkan::VkPhysicalDeviceMemoryProperties memProperties;
			Vulkan::vkGetPhysicalDeviceMemoryProperties(self.physicalDevice, &memProperties);

			/*constexpr auto properties = 
				Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
				| Vulkan::VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;*/

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
				if (typeFilter & (1 << i) and (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
					return i;

			throw std::runtime_error("Failed to find suitable memory type.");
		}

		auto FindQueueFamilies(this auto& self, Vulkan::VkPhysicalDevice device) -> QueueFamilyIndices
		{
			std::uint32_t queueFamilyCount = 0;
			Vulkan::vkGetPhysicalDeviceQueueFamilyProperties(
				device, 
				&queueFamilyCount, 
				nullptr
			);

			std::vector<Vulkan::VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			Vulkan::vkGetPhysicalDeviceQueueFamilyProperties(
				device, 
				&queueFamilyCount, 
				queueFamilies.data()
			);

			int i = 0;
			QueueFamilyIndices indices{};
			for (const auto& queueFamily : queueFamilies)
			{
				if (queueFamily.queueFlags & Vulkan::VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
					indices = { .GraphicsFamily = i };

				VkBool32 presentSupport = false;
				Vulkan::vkGetPhysicalDeviceSurfaceSupportKHR(
					device, 
					i, 
					self.surface, 
					&presentSupport
				);
				if (presentSupport)
					indices.PresentFamily = i;

				if (indices.IsComplete())
					break;
				i++;
			}

			return indices;
		}

		auto GetRequiredExtensions(this const auto& self) -> std::vector<const char*>
		{
			std::uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions =
				GLFW::glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector<const char*> extensions(
				glfwExtensions,
				glfwExtensions + glfwExtensionCount
			);
			if (EnableValidationLayers)
				extensions.push_back(Vulkan::DebugUtilExtensionName);
			return extensions;
		}

		void InitVulkan(this auto& self)
		{
			self.CreateInstance();
			self.SetupDebugMessenger();
			self.CreateSurface();
			self.PickPhysicalDevice();
			self.CreateLogicalDevice();
			self.CreateSwapChain();
			self.CreateImageViews();
			self.CreateRenderPass();
			self.CreateGraphicsPipeline();
			self.CreateFramebuffers();
			self.CreateCommandPool();
			self.CreateVertexBuffer();
			self.CreateIndexBuffer();
			self.CreateCommandBuffers();
			self.CreateSyncObjects();
		}

		static void FramebufferResizeCallback(GLFW::GLFWwindow* window, int width, int height) 
		{
			auto app = reinterpret_cast<Application*>(GLFW::glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		}

		void InitWindow(this auto& self)
		{
			GLFW::glfwInit();
			GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
			GLFW::glfwWindowHint(GLFW::Resizable, false);
			self.m_window = GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
			GLFW::glfwSetWindowUserPointer(self.m_window, &self);
			GLFW::glfwSetFramebufferSizeCallback(self.m_window, FramebufferResizeCallback);
		}

		auto IsDeviceSuitable(
			this const auto& self, 
			Vulkan::VkPhysicalDevice device
		) -> bool
		{
			QueueFamilyIndices indices = self.FindQueueFamilies(device);
			bool extensionsSupported = self.CheckDeviceExtensionSupport(device);
			bool swapChainAdequate = 
				[&] -> bool
				{
					if (not extensionsSupported)
						return false;
					SwapChainSupportDetails swapChainSupport =
						self.QuerySwapChainSupport(device);
					return not swapChainSupport.Formats.empty()
						and not swapChainSupport.PresentModes.empty();
				}();
			
			return indices.IsComplete()
				and extensionsSupported 
				and swapChainAdequate;
		}

		void MainLoop(this auto& self)
		{
			while (not GLFW::glfwWindowShouldClose(self.m_window))
			{
				GLFW::glfwPollEvents();
				self.DrawFrame();
			}

			Vulkan::vkDeviceWaitIdle(self.device);
		}

		auto PickPhysicalDevice(this auto& self) -> Vulkan::VkPhysicalDevice
		{
			std::uint32_t deviceCount = 0;
			Vulkan::vkEnumeratePhysicalDevices(self.m_instance, &deviceCount, nullptr);
			if (deviceCount == 0)
				throw std::runtime_error("Failed to find GPUs with Vulkan support.");

			std::vector<Vulkan::VkPhysicalDevice> devices(deviceCount);
			Vulkan::vkEnumeratePhysicalDevices(self.m_instance, &deviceCount, devices.data());

			for (const auto& device : devices)
				if (self.IsDeviceSuitable(device))
					return (self.physicalDevice = device, device);
			throw std::runtime_error("Failed to find a suitable GPU.");
		}

		void PopulateDebugMessengerCreateInfo(
			this auto& self, 
			Vulkan::VkDebugUtilsMessengerCreateInfoEXT& createInfo
		)
		{
			createInfo = Vulkan::VkDebugUtilsMessengerCreateInfoEXT{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = // filter messages based on severity
					Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
					| Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
					| Vulkan::VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = // filter messages based on type
					Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
					| Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
					| Vulkan::VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = DebugCallback,
				.pUserData = nullptr // Optional
			};
		}

		auto QuerySwapChainSupport(
			this const auto& self,
			Vulkan::VkPhysicalDevice device
		) -> SwapChainSupportDetails
		{
			SwapChainSupportDetails details;
			Vulkan::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				device,
				self.surface,
				&details.Capabilities
			);

			std::uint32_t formatCount;
			Vulkan::vkGetPhysicalDeviceSurfaceFormatsKHR(
				device,
				self.surface,
				&formatCount,
				details.Formats.data()
			);
			if (formatCount != 0)
			{
				details.Formats.resize(formatCount);
				Vulkan::vkGetPhysicalDeviceSurfaceFormatsKHR(
					device,
					self.surface,
					&formatCount,
					details.Formats.data()
				);
			}

			std::uint32_t presentModeCount;
			Vulkan::vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, 
				self.surface, 
				&presentModeCount, 
				nullptr
			);
			if (presentModeCount != 0)
			{
				details.PresentModes.resize(presentModeCount);
				Vulkan::vkGetPhysicalDeviceSurfacePresentModesKHR(
					device,
					self.surface,
					&presentModeCount,
					details.PresentModes.data()
				);
			}

			return details;
		}

		void RecreateSwapChain(this auto& self)
		{
			int width = 0, height = 0;
			GLFW::glfwGetFramebufferSize(self.m_window, &width, &height);
			while (width == 0 or height == 0) 
			{
				GLFW::glfwGetFramebufferSize(self.m_window, &width, &height);
				GLFW::glfwWaitEvents();
			}

			Vulkan::vkDeviceWaitIdle(self.device);

			self.CleanupSwapChain();
			self.CreateSwapChain();
			self.CreateImageViews();
			self.CreateFramebuffers();
		}

		void SetupDebugMessenger(this auto& self)
		{
			if (not EnableValidationLayers)
				return;
			Vulkan::VkDebugUtilsMessengerCreateInfoEXT createInfo{};

			self.PopulateDebugMessengerCreateInfo(createInfo);

			auto result = CreateDebugUtilsMessengerEXT(
				self.m_instance,
				&createInfo,
				nullptr,
				&self.m_debugMessenger
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to set up debug messenger");
		}

		void RecordCommandBuffer(
			this auto& self, 
			Vulkan::VkCommandBuffer commandBuffer, 
			std::uint32_t imageIndex
		)
		{
			Vulkan::VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			auto result = Vulkan::vkBeginCommandBuffer(commandBuffer, &beginInfo);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to begin recording command buffer.");

			Vulkan::VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = self.renderPass;
			renderPassInfo.framebuffer = self.swapChainFramebuffers[imageIndex];

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = self.swapChainExtent;

			Vulkan::VkClearValue clearColor{ {{0.0f, 0.0f, 0.0f, 1.0f}} };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			Vulkan::vkCmdBeginRenderPass(
				commandBuffer, 
				&renderPassInfo, 
				Vulkan::VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE
			);

			Vulkan::vkCmdBindPipeline(
				commandBuffer, 
				Vulkan::VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, 
				self.graphicsPipeline
			);

			Vulkan::VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(self.swapChainExtent.width);
			viewport.height = static_cast<float>(self.swapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			Vulkan::vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			Vulkan::VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = self.swapChainExtent;
			Vulkan::vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			Vulkan::VkBuffer vertexBuffers[] = { self.vertexBuffer };
			Vulkan::VkDeviceSize offsets[] = { 0 };
			Vulkan::vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			Vulkan::vkCmdBindIndexBuffer(commandBuffer, self.indexBuffer, 0, Vulkan::VkIndexType::VK_INDEX_TYPE_UINT16);

			Vulkan::vkCmdDrawIndexed(commandBuffer, static_cast<std::uint32_t>(self.indices.size()), 1, 0, 0, 0);

			Vulkan::vkCmdEndRenderPass(commandBuffer);
			if (Vulkan::vkEndCommandBuffer(commandBuffer) != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to record command buffer.");
		}
	};
}
