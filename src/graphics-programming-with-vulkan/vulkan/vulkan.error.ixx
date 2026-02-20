export module vulkangfx:vulkan.error;
import std;
import :vulkan.exports;

export namespace Vulkan
{
	struct Result
	{
		vkr::VkResult Value = vkr::VkResult::VK_SUCCESS;
		
		constexpr operator bool(this const Result& self) noexcept
		{
			return self.Value == vkr::VkResult::VK_SUCCESS;
		}

		constexpr auto Succeeded(this const Result& self) noexcept -> bool
		{
			return self.Value == vkr::VkResult::VK_SUCCESS;
		}

		constexpr auto Failed(this const Result& self) noexcept -> bool
		{
			return self.Value != vkr::VkResult::VK_SUCCESS;
		}

		auto Describe(this const Result& self) noexcept -> std::string_view
		{
			return vkr::VkResultToString(self.Value);
		}
	};

	class VulkanError : public std::runtime_error
	{
	public:
		explicit VulkanError(
			Result result,
			std::string_view message,
			const std::source_location& loc = std::source_location::current()
		) : std::runtime_error{ FormatMessage(message, result.Value, loc) }, result{ result.Value }
		{}

		explicit VulkanError(
			vkr::VkResult result, 
			std::string_view message,
			const std::source_location& loc = std::source_location::current()
		) : std::runtime_error{FormatMessage(message, result, loc)}, result{result} 
		{}

		auto GetResult(this const VulkanError& self) noexcept -> vkr::VkResult
		{
			return self.result;
		}

	private:
		vkr::VkResult result = vkr::VkResult::VK_SUCCESS;

	private:
		static auto FormatMessage(
			std::string_view message, 
			vkr::VkResult result,
			const std::source_location& location
		) noexcept -> std::string
		{
			return std::format(
				"{} (VkResult: {} - {}) [{}:{}:{}]", 
				message, 
				static_cast<int>(result), 
				vkr::VkResultToString(result),
				location.file_name(),
				location.line(),
				location.column()
			);
		}
	};
}
