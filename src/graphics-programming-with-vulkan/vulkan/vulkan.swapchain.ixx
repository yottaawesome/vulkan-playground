export module vulkangfx:vulkan.swapchain;
import std;
import :error;
import :vulkan.exports;
import :vulkan.error;

export namespace Vulkan
{
	struct SwapchainDeleter
	{
	public:
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
			if (swapchain)
				vkr::vkDestroySwapchainKHR(self.Device, swapchain, nullptr);
		}
	private:
		vkr::VkDevice Device = nullptr;
	};
	using VkSwapchainKHRUniquePtr = std::unique_ptr<std::remove_pointer_t<vkr::VkSwapchainKHR>, SwapchainDeleter>;

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
			auto swapchain = vkr::VkSwapchainKHR{ nullptr };
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
			return std::forward_like<decltype(self)>(self.Image).begin(); 
		}

		constexpr auto end(this auto&& self) noexcept
		{
			return std::forward_like<decltype(self)>(self.Image).end(); 
		}

		constexpr auto count(this const SwapchainImages& self) noexcept
		{
			return static_cast<std::uint32_t>(self.Images.size());
		}

		constexpr auto operator[](this auto&& self, std::size_t index)
		{
			if (index >= self.Image.size())
				throw std::out_of_range{ "Index out of range in SwapchainImageViews." };
			return std::forward_like<decltype(self)>(self.Image)[index];
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
		Swapchain(VkSwapchainKHRUniquePtr handleIn, vkr::VkDevice deviceIn)
			: handle(std::move(handleIn)), device(deviceIn)
		{
			if (not device)
				throw Error::RuntimeError{ "Device must not be null to create a swapchain." };
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
				self.device,
				self.handle.get(),
				&count,
				nullptr
			) };
			if (not result)
				throw VulkanError{ result, "Failed to get swapchain image count." };

			auto images = std::vector<vkr::VkImage>(count);
			result = Result{
				vkr::vkGetSwapchainImagesKHR(
					self.device,
					self.handle.get(),
					&count,
					images.data()
				)};
			if (not result)
				throw VulkanError{ result, "Failed to get swapchain images." };

			return SwapchainImages{ std::move(images) };
		}

	private:
		VkSwapchainKHRUniquePtr handle;
		vkr::VkDevice device{};
	};
}