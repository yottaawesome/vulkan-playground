export module vulkan26:vulkan.swapchain;

import std;
import :error;
import :vulkan.error;
import :vulkan.exports;

export namespace vk
{
	class SwapchainDeleter
	{
	public:
		SwapchainDeleter(VkDevice deviceIn)
			: device(deviceIn)
		{
			if (not device)
				throw ::Error::RuntimeError{ "Device must not be null." };
		}

		void operator()(VkSwapchainKHR swapchain) const noexcept
		{
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}

		constexpr auto GetDevice(this const auto& self) noexcept -> VkDevice
		{
			return self.device;
		}

	private:
		VkDevice device = nullptr;
	};
	using SwapchainUniquePtr = std::unique_ptr<std::remove_pointer_t<VkSwapchainKHR>, SwapchainDeleter>;

	template<typename TSwapchainUniquePtr>
	class TypedSwapchain
	{
	public:
		constexpr TypedSwapchain(TSwapchainUniquePtr swapchainIn)
			: swapchain(std::move(swapchainIn))
		{
			if (not swapchain)
				throw ::Error::RuntimeError{ "Swapchain handle must not be null." };
		}

		constexpr void Destroy(this auto& self) noexcept
		{
			self.swapchain.reset();
		}

		constexpr auto Get(this const auto& self) noexcept -> typename TSwapchainUniquePtr::pointer
		{
			return self.swapchain.get();
		}

		constexpr auto operator*(this const auto& self) noexcept -> typename TSwapchainUniquePtr::pointer
		{
			return self.swapchain.get();
		}

		auto GetSwapchainImages(this const auto& self) -> std::vector<VkImage>
		{
			auto device = self.swapchain.get_deleter().GetDevice();
			auto swapchainImagesCount = uint32_t{};
			auto result = Result{ vkGetSwapchainImagesKHR(device, self.Get(), &swapchainImagesCount, nullptr) };
			if (not result)
				throw Error{ result.result };
			auto swapchainImages = std::vector<VkImage>(swapchainImagesCount);
			result = vkGetSwapchainImagesKHR(device, self.Get(), &swapchainImagesCount, swapchainImages.data());
			if (not result)
				throw Error{ result.result };
			return swapchainImages;
		}

	private:
		TSwapchainUniquePtr swapchain;
	};
	static_assert(
		[] consteval -> bool
		{
			auto swapchain = TypedSwapchain<std::unique_ptr<int>>{ std::make_unique<int>(42) };
			if (*swapchain.Get() != 42)
				throw "Swapchain handle test failed.";
			if (**swapchain != 42)
				throw "Swapchain operator* test failed.";
			swapchain.Destroy();
			if (swapchain.Get() != nullptr)
				throw "Swapchain destroy test failed.";
			return true;
		}());

	using Swapchain = TypedSwapchain<SwapchainUniquePtr>;
}
