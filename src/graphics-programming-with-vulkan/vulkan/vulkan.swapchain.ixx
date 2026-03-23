export module vulkangfx:vulkan.swapchain;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	struct SwapchainDeleter
	{
		vkr::VkDevice Device = nullptr;
		SwapchainDeleter(const SwapchainDeleter&) = delete;
		SwapchainDeleter& operator=(const SwapchainDeleter&) = delete;

		SwapchainDeleter(SwapchainDeleter&&) = default;
		SwapchainDeleter& operator=(SwapchainDeleter&&) = default;

		SwapchainDeleter(vkr::VkDevice device)
			: Device(device)
		{
			if (not Device)
				throw Error::RuntimeError{ "Device must not be null for SwapchainDeleter." };
		}

		void operator()(this SwapchainDeleter& self, vkr::VkSwapchainKHR swapchain) noexcept
		{
			vkr::vkDestroySwapchainKHR(self.Device, swapchain, nullptr);
		}
	};
	using VkSwapchainKHRUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkSwapchainKHR>, SwapchainDeleter>;

	struct SwapchainCapabilities
	{
		vkr::VkSurfaceCapabilitiesKHR Capabilities{};
		std::vector<vkr::VkSurfaceFormatKHR> Formats;
		std::vector<vkr::VkPresentModeKHR> PresentModes;

		static auto Query(vkr::VkPhysicalDevice device, vkr::VkSurfaceKHR surface) -> SwapchainCapabilities
		{
			auto capabilities = SwapchainCapabilities{};

			if (not device)
				throw Error::RuntimeError{ "Physical device must not be null to query swapchain capabilities." };
			if (not surface)
				throw Error::RuntimeError{ "Surface must not be null to query swapchain capabilities." };

			// Query surface capabilities.
			auto result = Result{ vkr::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities.Capabilities) };
			if (not result)
				throw VulkanError{ result, "Failed to query surface capabilities." };
			auto formatCount = std::uint32_t{};
			result = Result{ vkr::vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr) };
			if (not result)
				throw VulkanError{ result, "Failed to query surface formats count." };

			// Query surface formats.
			if (formatCount > 0)
			{
				capabilities.Formats.resize(formatCount);
				result = Result{ vkr::vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, capabilities.Formats.data()) };
				if (not result)
					throw VulkanError{ result, "Failed to query surface formats." };
			}

			//Query present modes count.
			auto presentModeCount = std::uint32_t{};
			result = Result{ vkr::vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr) };
			if (not result)
				throw VulkanError{ result, "Failed to query present modes count." };
			if (presentModeCount > 0)
			{
				capabilities.PresentModes.resize(presentModeCount);
				result = Result{ vkr::vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, capabilities.PresentModes.data()) };
				if (not result)
					throw VulkanError{ result, "Failed to query present modes." };
			}
			return capabilities;
		}
	};

	struct SwapchainFactory
	{
		// https://docs.vulkan.org/refpages/latest/refpages/source/VkSwapchainCreateInfoKHR.html
		vkr::VkSwapchainCreateInfoKHR Info{};
		vkr::VkDevice Device = nullptr;

		[[nodiscard]]
		auto operator()(this SwapchainFactory& self) -> VkSwapchainKHRUniquePtr
		{
			if (not self.Device)
				throw Error::RuntimeError{ "Device must not be null to create a swapchain." };

			self.Info.sType = vkr::VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			auto swapchain = vkr::VkSwapchainKHR{ };
			auto result = Result{ vkr::vkCreateSwapchainKHR(self.Device, &self.Info, nullptr, &swapchain) };
			if (not result)
				throw VulkanError{ result, "Failed to create swapchain." };
			return VkSwapchainKHRUniquePtr(swapchain, SwapchainDeleter(self.Device));
		}
	};

	struct SwapchainImages
	{
		constexpr SwapchainImages() = default;

		SwapchainImages(std::vector<vkr::VkImage> images)
			: Images(std::move(images))
		{ }

		constexpr auto begin(this auto&& self) noexcept
		{ 
			return std::forward_like<decltype(self)>(self.Images).begin(); 
		}

		constexpr auto end(this auto&& self) noexcept
		{
			return std::forward_like<decltype(self)>(self.Images).end(); 
		}

		constexpr auto size(this const SwapchainImages& self) noexcept
		{
			return static_cast<std::uint32_t>(self.Images.size());
		}

		constexpr auto operator[](this auto&& self, std::size_t index)
		{
			if (index >= self.Images.size())
				throw std::out_of_range{ "Index out of range in SwapchainImageViews." };
			return std::forward_like<decltype(self)>(self.Images)[index];
		}

		constexpr auto GetImages(this auto&& self) noexcept
		{
			return std::forward_like<decltype(self)>(self.Image);
		}

		std::vector<vkr::VkImage> Images;
	};

	class Swapchain
	{
	public:
		Swapchain(VkSwapchainKHRUniquePtr handleIn)
			: handle(std::move(handleIn))
		{
			if (not handle)
				throw Error::RuntimeError{ "Swapchain handle must not be null." };
		}

		[[nodiscard]]
		constexpr auto GetHandle(this const Swapchain& self) noexcept -> vkr::VkSwapchainKHR
		{
			return self.handle.get();
		}

		[[nodiscard]]
		auto GetImages(this const Swapchain& self) -> SwapchainImages
		{
			auto count = std::uint32_t{};
			auto result = Result{
				vkr::vkGetSwapchainImagesKHR(
				self.handle.get_deleter().Device,
				self.handle.get(),
				&count,
				nullptr
			) };
			if (not result)
				throw VulkanError{ result, "Failed to get swapchain image count." };

			auto images = std::vector<vkr::VkImage>(count);
			result = Result{
				vkr::vkGetSwapchainImagesKHR(
					self.handle.get_deleter().Device,
					self.handle.get(),
					&count,
					images.data()
				)};
			if (not result)
				throw VulkanError{ result, "Failed to get swapchain images." };

			return SwapchainImages{ std::move(images) };
		}

		struct NextSwapchainImage
		{
			Result Result{};
			std::uint32_t ImageIndex = 0;
		};
		auto AcquireNextImage(
			this const Swapchain& self,
			vkr::VkSemaphore semaphoreToSignal = nullptr,
			vkr::VkFence fenceToSignal = nullptr,
			std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()
		) -> NextSwapchainImage
		{
			auto imageIndex = std::uint32_t{};
			auto result = Result{
				vkr::vkAcquireNextImageKHR(
					self.handle.get_deleter().Device,
					self.handle.get(),
					timeout,
					semaphoreToSignal,
					fenceToSignal,
					&imageIndex
				)};
			return { result, imageIndex };
		}

	private:
		VkSwapchainKHRUniquePtr handle;
	};
}