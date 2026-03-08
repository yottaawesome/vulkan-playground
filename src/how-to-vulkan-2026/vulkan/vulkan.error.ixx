export module vulkan26:vulkan.error;
import std;
import :vulkan.exports;

export namespace vk
{
	struct Error : std::runtime_error
	{
		const VkResult result;
		Error(
			VkResult result, 
			const std::source_location& loc = std::source_location::current()
		) noexcept
			: result(result)
			, std::runtime_error(Format(result, loc))
		{ }

		static auto Format(VkResult result, const std::source_location& loc) -> std::string
		{
			return std::format("Vulkan error: {} at {}:{}", static_cast<std::uint32_t>(result), loc.file_name(), loc.line());
		}
	};

	struct Result
	{
		VkResult result = VkResult::VK_SUCCESS;
		constexpr Result(VkResult result) noexcept : result(result) {}
		constexpr operator bool(this const auto& self) noexcept
		{
			return self.result == VkResult::VK_SUCCESS;
		}
	};
}