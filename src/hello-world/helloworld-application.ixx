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

	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static auto GetBindingDescription() -> Vulkan::VkVertexInputBindingDescription
		{
			Vulkan::VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = Vulkan::VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static auto GetAttributeDescriptions() -> std::array<Vulkan::VkVertexInputAttributeDescription, 3>
		{
			std::array<Vulkan::VkVertexInputAttributeDescription, 3> attributeDescriptions{};

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
			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = 
				((size_t)&reinterpret_cast<char const volatile&>((((Vertex*)0)->texCoord)));

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

		void Run(this Application& self)
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
		Vulkan::VkDescriptorSetLayout descriptorSetLayout;
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
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
		};
		const std::vector<std::uint16_t> indices = {
			0, 1, 2, 2, 3, 0
		};
		Vulkan::VkBuffer vertexBuffer;
		Vulkan::VkDeviceMemory vertexBufferMemory;
		Vulkan::VkBuffer indexBuffer;
		Vulkan::VkDeviceMemory indexBufferMemory;
		std::vector<Vulkan::VkBuffer> uniformBuffers;
		std::vector<Vulkan::VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
		Vulkan::VkDescriptorPool descriptorPool;
		std::vector<Vulkan::VkDescriptorSet> descriptorSets;
		Vulkan::VkImage textureImage;
		Vulkan::VkDeviceMemory textureImageMemory;
		Vulkan::VkImageView textureImageView;
		Vulkan::VkSampler textureSampler;

		auto BeginSingleTimeCommands(this Application& self) -> Vulkan::VkCommandBuffer
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

			return commandBuffer;
		}

		void EndSingleTimeCommands(this Application& self, Vulkan::VkCommandBuffer commandBuffer)
		{
			Vulkan::vkEndCommandBuffer(commandBuffer);

			Vulkan::VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			Vulkan::vkQueueSubmit(self.graphicsQueue, 1, &submitInfo, nullptr);
			Vulkan::vkQueueWaitIdle(self.graphicsQueue);
			Vulkan::vkFreeCommandBuffers(self.device, self.commandPool, 1, &commandBuffer);
		}

		auto CheckDeviceExtensionSupport(
			this const Application& self,
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

		auto CheckValidationLayerSupport(this const Application& self) -> bool
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
			this const Application& self,
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
			this const Application& self,
			const std::vector<Vulkan::VkPresentModeKHR>& availablePresentModes
		) -> Vulkan::VkPresentModeKHR
		{
			for (auto&& mode : availablePresentModes)
				if (mode == Vulkan::VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
					return mode;
			return Vulkan::VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		}

		auto ChooseSwapSurfaceFormat(
			this const Application& self,
			const std::vector<Vulkan::VkSurfaceFormatKHR>& availableFormats
		) -> Vulkan::VkSurfaceFormatKHR
		{
			for (const auto& availableFormat : availableFormats)
				if (availableFormat.format == Vulkan::VkFormat::VK_FORMAT_B8G8R8A8_SRGB
					and availableFormat.colorSpace == Vulkan::VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
					) return availableFormat;
			return availableFormats[0];
		}

		void Cleanup(this Application& self)
		{
			self.CleanupSwapChain();

			Vulkan::vkDestroySampler(self.device, self.textureSampler, nullptr);
			Vulkan::vkDestroyImageView(self.device, self.textureImageView, nullptr);
			Vulkan::vkDestroyImage(self.device, self.textureImage, nullptr);
			Vulkan::vkFreeMemory(self.device, self.textureImageMemory, nullptr);

			for (size_t i = 0; i < MaxFramesInFlight; i++)
			{
				Vulkan::vkDestroyBuffer(self.device, self.uniformBuffers[i], nullptr);
				Vulkan::vkFreeMemory(self.device, self.uniformBuffersMemory[i], nullptr);
			}
			Vulkan::vkDestroyDescriptorPool(self.device, self.descriptorPool, nullptr);
			Vulkan::vkDestroyDescriptorSetLayout(self.device, self.descriptorSetLayout, nullptr);

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

		void CleanupSwapChain(this Application& self)
		{
			for (auto framebuffer : self.swapChainFramebuffers)
				Vulkan::vkDestroyFramebuffer(self.device, framebuffer, nullptr);
			for (auto imageView : self.swapChainImageViews)
				Vulkan::vkDestroyImageView(self.device, imageView, nullptr);
			Vulkan::vkDestroySwapchainKHR(self.device, self.swapChain, nullptr);
		}

		void CopyBuffer(
			this Application& self,
			Vulkan::VkBuffer srcBuffer,
			Vulkan::VkBuffer dstBuffer,
			Vulkan::VkDeviceSize size
		)
		{
			Vulkan::VkCommandBuffer commandBuffer = self.BeginSingleTimeCommands();

			Vulkan::VkBufferCopy copyRegion{};
			copyRegion.size = size;
			Vulkan::vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

			self.EndSingleTimeCommands(commandBuffer);
		}

		void CopyBufferToImage(
			this Application& self, 
			Vulkan::VkBuffer buffer, 
			Vulkan::VkImage image,
			std::uint32_t width,
			std::uint32_t height
		)
		{
			Vulkan::VkCommandBuffer commandBuffer = self.BeginSingleTimeCommands();
			Vulkan::VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;

			region.imageSubresource.aspectMask = Vulkan::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;

			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = {
				width,
				height,
				1
			};
			Vulkan::vkCmdCopyBufferToImage(
				commandBuffer,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
			);
			self.EndSingleTimeCommands(commandBuffer);
		}

		void CreateBuffer(
			this Application& self,
			Vulkan::VkDeviceSize size,
			Vulkan::VkBufferUsageFlags usage,
			Vulkan::VkMemoryPropertyFlags properties,
			Vulkan::VkBuffer& buffer,
			Vulkan::VkDeviceMemory& bufferMemory
		)
		{
			Vulkan::VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = size;
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
				throw std::runtime_error("Failed to allocate buffer memory.");

			if (Vulkan::vkBindBufferMemory(self.device, buffer, bufferMemory, 0) != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to allocate bind buffer memory.");
		}

		void CreateCommandBuffers(this Application& self)
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

		void CreateCommandPool(this Application& self)
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

		void CreateDescriptorPool(this Application& self)
		{
			std::array<Vulkan::VkDescriptorPoolSize, 2> poolSizes{};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(MaxFramesInFlight);
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = static_cast<uint32_t>(MaxFramesInFlight);

			Vulkan::VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<std::uint32_t>(MaxFramesInFlight);

			if (Vulkan::vkCreateDescriptorPool(self.device, &poolInfo, nullptr, &self.descriptorPool) != VK_SUCCESS)
				throw std::runtime_error("Failed to create descriptor pool.");
		}

		auto CreateDescriptorSetLayout(this Application& self)
		{
			Vulkan::VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = Vulkan::VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = Vulkan::VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

			Vulkan::VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			samplerLayoutBinding.binding = 1;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
			Vulkan::VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			if (Vulkan::vkCreateDescriptorSetLayout(self.device, &layoutInfo, nullptr, &self.descriptorSetLayout) != VK_SUCCESS)
				throw std::runtime_error("failed to create descriptor set layout!");
		}

		void CreateDescriptorSets(this Application& self)
		{
			std::vector<Vulkan::VkDescriptorSetLayout> layouts(MaxFramesInFlight, self.descriptorSetLayout);
			Vulkan::VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = self.descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(MaxFramesInFlight);
			allocInfo.pSetLayouts = layouts.data();

			self.descriptorSets.resize(MaxFramesInFlight);
			if (Vulkan::vkAllocateDescriptorSets(
				self.device,
				&allocInfo,
				self.descriptorSets.data()) != VK_SUCCESS
			)
			{
				throw std::runtime_error("Failed to allocate descriptor sets.");
			}

			for (size_t i = 0; i < MaxFramesInFlight; i++)
			{
				Vulkan::VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = self.uniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = Vulkan::WholeSize; //
				// sizeof(UniformBufferObject); gives validation errors.

				Vulkan::VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = self.textureImageView;
				imageInfo.sampler = self.textureSampler;

				std::array<Vulkan::VkWriteDescriptorSet, 2> descriptorWrites{};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = self.descriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = self.descriptorSets[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				Vulkan::vkUpdateDescriptorSets(
					self.device, 
					static_cast<uint32_t>(descriptorWrites.size()), 
					descriptorWrites.data(), 
					0, 
					nullptr
				);
			}
		}

		void CreateFramebuffers(this Application& self)
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

		void CreateGraphicsPipeline(this Application& self)
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
				.setLayoutCount = 1,
				.pSetLayouts = &self.descriptorSetLayout,
				.pushConstantRangeCount = 0, // Optional
				.pPushConstantRanges = nullptr // Optional
			};

			auto result = Vulkan::vkCreatePipelineLayout(
				self.device,
				&pipelineLayoutInfo,
				nullptr,
				&self.pipelineLayout
			);
			if (result != VK_SUCCESS)
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
			rasterizer.frontFace = Vulkan::VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

		auto CreateImageView(this Application& self, VkImage image, VkFormat format) -> Vulkan::VkImageView
		{
			Vulkan::VkImageViewCreateInfo viewInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = image,
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = format,
				.subresourceRange{ 
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1 
				}
			};
			Vulkan::VkImageView imageView;
			if (Vulkan::vkCreateImageView(self.device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image view.");

			return imageView;
		}

		void CreateImageViews(this Application& self)
		{
			self.swapChainImageViews.resize(self.swapChainImages.size());
			for (size_t i = 0; i < self.swapChainImages.size(); i++)
				self.swapChainImageViews[i] = 
					self.CreateImageView(self.swapChainImages[i], self.swapChainImageFormat);
		}

		void CreateIndexBuffer(this Application& self)
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

		void CreateInstance(this Application& self)
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

		void CreateLogicalDevice(this Application& self)
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
				Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = queueFamily,
					.queueCount = 1,
					.pQueuePriorities = &queuePriority
				};
				queueCreateInfos.push_back(queueCreateInfo);
			}

			Vulkan::VkDeviceQueueCreateInfo queueCreateInfo{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = indices.GraphicsFamily.value(),
				.queueCount = 1
			};
			queueCreateInfo.pQueuePriorities = &queuePriority;

			Vulkan::VkPhysicalDeviceFeatures deviceFeatures{
				.samplerAnisotropy = true
			};
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

		void CreateRenderPass(this Application& self)
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

		auto CreateShaderModule(
			this Application& self,
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

		void CreateSurface(this Application& self)
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

		void CreateSwapChain(this Application& self)
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

		void CreateSyncObjects(this Application& self)
		{
			self.imageAvailableSemaphores.resize(MaxFramesInFlight);
			self.renderFinishedSemaphores.resize(self.swapChainImages.size());
			self.inFlightFences.resize(MaxFramesInFlight);

			Vulkan::VkSemaphoreCreateInfo semaphoreInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};
			Vulkan::VkFenceCreateInfo fenceInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = Vulkan::VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT
			};

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

		void CreateImage(
			this Application& self, 
			uint32_t width, 
			uint32_t height, 
			Vulkan::VkFormat format, 
			Vulkan::VkImageTiling tiling,
			Vulkan::VkImageUsageFlags usage,
			Vulkan::VkMemoryPropertyFlags properties,
			Vulkan::VkImage& image,
			Vulkan::VkDeviceMemory& imageMemory
		) 
		{
			Vulkan::VkImageCreateInfo imageInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format,
				.extent{
					.width = width,
					.height = height,
					.depth = 1
				},
				.mipLevels = 1,
				.arrayLayers = 1,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.tiling = tiling,
				.usage = usage,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
			};
			if (Vulkan::vkCreateImage(self.device, &imageInfo, nullptr, &image) != VK_SUCCESS)
				throw std::runtime_error("Failed to create image.");

			Vulkan::VkMemoryRequirements memRequirements;
			Vulkan::vkGetImageMemoryRequirements(self.device, image, &memRequirements);

			Vulkan::VkMemoryAllocateInfo allocInfo{
				.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				.allocationSize = memRequirements.size,
				.memoryTypeIndex = self.FindMemoryType(memRequirements.memoryTypeBits, properties),
			};
			if (Vulkan::vkAllocateMemory(self.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) 
				throw std::runtime_error("Failed to allocate image memory.");

			Vulkan::vkBindImageMemory(self.device, image, imageMemory, 0);
		}

		void CreateTextureImage(this Application& self)
		{
			int texWidth, texHeight, texChannels;
			stb::stbi_uc* pixels = stb::stbi_load(
				"textures/texture.jpg", 
				&texWidth, 
				&texHeight, 
				&texChannels, 
				stb::STBI_rgb_alpha
			);
			Vulkan::VkDeviceSize imageSize = texWidth * texHeight * 4;

			if (!pixels)
				throw std::runtime_error("Failed to load texture image.");

			Vulkan::VkBuffer stagingBuffer;
			Vulkan::VkDeviceMemory stagingBufferMemory;
			self.CreateBuffer(
				imageSize, 
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				stagingBuffer, 
				stagingBufferMemory
			);

			void* data;
			Vulkan::vkMapMemory(self.device, stagingBufferMemory, 0, imageSize, 0, &data);
			std::memcpy(data, pixels, static_cast<size_t>(imageSize));
			Vulkan::vkUnmapMemory(self.device, stagingBufferMemory);

			stb::stbi_image_free(pixels);

			self.CreateImage(
				texWidth, 
				texHeight, 
				VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				self.textureImage, 
				self.textureImageMemory
			);

			self.TransitionImageLayout(
				self.textureImage, 
				VK_FORMAT_R8G8B8A8_SRGB, 
				VK_IMAGE_LAYOUT_UNDEFINED, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			);
			self.CopyBufferToImage(
				stagingBuffer, 
				self.textureImage, 
				static_cast<uint32_t>(texWidth), 
				static_cast<uint32_t>(texHeight)
			);

			self.TransitionImageLayout(
				self.textureImage, 
				VK_FORMAT_R8G8B8A8_SRGB, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			);

			Vulkan::vkDestroyBuffer(self.device, stagingBuffer, nullptr);
			Vulkan::vkFreeMemory(self.device, stagingBufferMemory, nullptr);
		}

		void CreateTextureImageView(this Application& self) 
		{
			self.textureImageView = self.CreateImageView(self.textureImage, VK_FORMAT_R8G8B8A8_SRGB);
		}

		void CreateTextureSampler(this Application& self)
		{
			Vulkan::VkPhysicalDeviceProperties properties{};
			Vulkan::vkGetPhysicalDeviceProperties(self.physicalDevice, &properties);

			Vulkan::VkSamplerCreateInfo samplerInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
				.magFilter = Vulkan::VkFilter::VK_FILTER_LINEAR,
				.minFilter = Vulkan::VkFilter::VK_FILTER_LINEAR,
				.mipmapMode = Vulkan::VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR,
				.addressModeU = Vulkan::VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.addressModeV = Vulkan::VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.addressModeW = Vulkan::VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT,
				.mipLodBias = 0.0f,
				.anisotropyEnable = true,
				.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
				.compareEnable = false,
				.compareOp = Vulkan::VkCompareOp::VK_COMPARE_OP_ALWAYS,
				.minLod = 0.0f,
				.maxLod = 0.0f,
				.borderColor = Vulkan::VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK,
				.unnormalizedCoordinates = false
			};

			if (Vulkan::vkCreateSampler(self.device, &samplerInfo, nullptr, &self.textureSampler) != VK_SUCCESS)
				throw std::runtime_error("failed to create texture sampler!");
		}

		void CreateUniformBuffers(this Application& self)
		{
			Vulkan::VkDeviceSize bufferSize = sizeof(UniformBufferObject);

			self.uniformBuffers.resize(MaxFramesInFlight);
			self.uniformBuffersMemory.resize(MaxFramesInFlight);
			self.uniformBuffersMapped.resize(MaxFramesInFlight);

			for (size_t i = 0; i < MaxFramesInFlight; i++)
			{
				self.CreateBuffer(
					bufferSize,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					self.uniformBuffers[i],
					self.uniformBuffersMemory[i]
				);
				Vulkan::vkMapMemory(self.device, self.uniformBuffersMemory[i], 0, bufferSize, 0, &self.uniformBuffersMapped[i]);
			}
		}

		void CreateVertexBuffer(this Application& self)
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
				std::println("Validation layer: {}", pCallbackData->pMessage);
			return false;
		}

		void DrawFrame(this Application& self)
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
			// Before or after the above?
			self.UpdateUniformBuffer(self.currentFrame);

			Vulkan::VkSemaphore waitSemaphores[]{ self.imageAvailableSemaphores[self.currentFrame] };
			Vulkan::VkPipelineStageFlags waitStages[]{
				Vulkan::VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			};
			Vulkan::VkSemaphore signalSemaphores[]{ self.renderFinishedSemaphores[imageIndex] };
			Vulkan::VkSubmitInfo submitInfo{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = waitSemaphores,
				.pWaitDstStageMask = waitStages,
				.commandBufferCount = 1,
				.pCommandBuffers = &self.commandBuffers[self.currentFrame],
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = signalSemaphores
			};
			result = Vulkan::vkQueueSubmit(
				self.graphicsQueue,
				1,
				&submitInfo,
				self.inFlightFences[self.currentFrame]
			);
			if (result != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to submit draw command buffer.");

			Vulkan::VkSwapchainKHR swapChains[] = { self.swapChain };
			Vulkan::VkPresentInfoKHR presentInfo{
				.sType = Vulkan::VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = signalSemaphores,
				.swapchainCount = 1,
				.pSwapchains = swapChains,
				.pImageIndices = &imageIndex,
				.pResults = nullptr // Optional
			};
			result = Vulkan::vkQueuePresentKHR(self.presentQueue, &presentInfo);
			if (
				result == VK_ERROR_OUT_OF_DATE_KHR 
				or result == VK_SUBOPTIMAL_KHR 
				or self.framebufferResized
			)
			{
				self.framebufferResized = false;
				self.RecreateSwapChain();
			}
			else if (result != VK_SUCCESS)
				throw std::runtime_error("Failed to present swap chain image.");

			self.currentFrame = (self.currentFrame + 1) % MaxFramesInFlight;
		}

		void EnumerateExtensions(this const Application& self)
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

		auto FindMemoryType(this const Application& self, std::uint32_t typeFilter, std::uint32_t properties) -> std::uint32_t
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

		auto FindQueueFamilies(this const Application& self, Vulkan::VkPhysicalDevice device) -> QueueFamilyIndices
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

		static void FramebufferResizeCallback(GLFW::GLFWwindow* window, int width, int height)
		{
			auto app = reinterpret_cast<Application*>(GLFW::glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		}

		auto GetRequiredExtensions(this const Application& self) -> std::vector<const char*>
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

		void InitVulkan(this Application& self)
		{
			self.CreateInstance();
			self.SetupDebugMessenger();
			self.CreateSurface();
			self.PickPhysicalDevice();
			self.CreateLogicalDevice();
			self.CreateSwapChain();
			self.CreateImageViews();
			self.CreateRenderPass();
			self.CreateDescriptorSetLayout();
			self.CreateGraphicsPipeline();
			self.CreateFramebuffers();
			self.CreateCommandPool();
			self.CreateTextureImage();
			self.CreateTextureImageView();
			self.CreateTextureSampler();
			self.CreateVertexBuffer();
			self.CreateIndexBuffer();
			self.CreateUniformBuffers();
			self.CreateDescriptorPool();
			self.CreateDescriptorSets();
			self.CreateCommandBuffers();
			self.CreateSyncObjects();
		}

		void InitWindow(this Application& self)
		{
			GLFW::glfwInit();
			GLFW::glfwWindowHint(GLFW::ClientApi, GLFW::NoApi);
			GLFW::glfwWindowHint(GLFW::Resizable, false);
			self.m_window = GLFW::glfwCreateWindow(Width, Height, "Vulkan", nullptr, nullptr);
			GLFW::glfwSetWindowUserPointer(self.m_window, &self);
			GLFW::glfwSetFramebufferSizeCallback(self.m_window, FramebufferResizeCallback);
		}

		auto IsDeviceSuitable(
			this const Application& self,
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

			Vulkan::VkPhysicalDeviceFeatures supportedFeatures;
			Vulkan::vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
			return indices.IsComplete()
				and extensionsSupported
				and swapChainAdequate 
				and supportedFeatures.samplerAnisotropy;
		}

		void MainLoop(this Application& self)
		{
			while (not GLFW::glfwWindowShouldClose(self.m_window))
			{
				GLFW::glfwPollEvents();
				self.DrawFrame();
			}

			Vulkan::vkDeviceWaitIdle(self.device);
		}

		auto PickPhysicalDevice(this Application& self) -> Vulkan::VkPhysicalDevice
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
			this Application& self,
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
			this const Application& self,
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

		void RecreateSwapChain(this Application& self)
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

		void RecordCommandBuffer(
			this Application& self,
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

			Vulkan::vkCmdBindDescriptorSets(
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				self.pipelineLayout,
				0,
				1,
				&self.descriptorSets[self.currentFrame],
				0,
				nullptr
			);
			Vulkan::vkCmdDrawIndexed(commandBuffer, static_cast<std::uint32_t>(self.indices.size()), 1, 0, 0, 0);

			Vulkan::vkCmdEndRenderPass(commandBuffer);
			if (Vulkan::vkEndCommandBuffer(commandBuffer) != Vulkan::VkResult::VK_SUCCESS)
				throw std::runtime_error("Failed to record command buffer.");
		}

		void SetupDebugMessenger(this Application& self)
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

		void TransitionImageLayout(
			this Application& self,
			Vulkan::VkImage image,
			Vulkan::VkFormat format,
			Vulkan::VkImageLayout oldLayout,
			Vulkan::VkImageLayout newLayout
		)
		{
			Vulkan::VkCommandBuffer commandBuffer = self.BeginSingleTimeCommands();

			Vulkan::VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = Vulkan::QueueFamilyIgnored;
			barrier.dstQueueFamilyIndex = Vulkan::QueueFamilyIgnored;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = Vulkan::VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0; // TODO
			barrier.dstAccessMask = 0; // TODO

			Vulkan::VkPipelineStageFlags sourceStage;
			Vulkan::VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
			{
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
			{
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else 
			{
				throw std::invalid_argument("Unsupported layout transition.");
			}

			Vulkan::vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, 
				destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1,
				&barrier
			);

			self.EndSingleTimeCommands(commandBuffer);
		}

		void UpdateUniformBuffer(this Application& self, uint32_t currentImage)
		{
			static auto startTime = std::chrono::high_resolution_clock::now();

			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			UniformBufferObject ubo{};
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f), self.swapChainExtent.width / (float)self.swapChainExtent.height, 0.1f, 10.0f);
			ubo.proj[1][1] *= -1;
			std::memcpy(self.uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
		}
	};
}