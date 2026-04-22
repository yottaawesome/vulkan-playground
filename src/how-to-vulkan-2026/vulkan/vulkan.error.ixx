export module vulkan26:vulkan.error;
import std;
import :vulkan.exports;

export namespace vk
{
	struct Result
	{
		VkResult Value = VkResult::VK_SUCCESS;
		constexpr Result() = default;
		constexpr Result(VkResult result) noexcept : Value(result) {}
		constexpr operator bool(this const auto& self) noexcept
		{
			return self.Value == VkResult::VK_SUCCESS;
		}
		constexpr auto operator==(this const auto& self, VkResult result)
		{
			return self.Value == result;
		}
	};
	static_assert(
		[] consteval -> bool
		{
			auto result = Result{};
			if (not result)
				throw "Expected default constructible Result to be VK_SUCCESS";
			result = VkResult::VK_ERROR_DEVICE_LOST;
			if (result != VkResult::VK_ERROR_DEVICE_LOST)
				throw "Expected value to be VK_ERROR_DEVICE_LOST.";
			return true;
		}());

	struct Error : std::runtime_error
	{
		const VkResult Code = VkResult::VK_ERROR_UNKNOWN;
		Error(VkResult result, const std::source_location& loc = std::source_location::current()) noexcept
			: Code(result), std::runtime_error(Format(result, loc, std::stacktrace::current(1)))
		{ }
		Error(Result result, const std::source_location& loc = std::source_location::current()) noexcept 
			: Code(result.Value), std::runtime_error(Format(result.Value, loc, std::stacktrace::current(1)))
		{ }

		static auto Format(VkResult result, const std::source_location& loc, const std::stacktrace& stack) -> std::string
		{
			return std::format("Vulkan error: {} at {}:{} at {}", static_cast<std::uint32_t>(result), loc.file_name(), loc.line(), stack);
		}
	};
}