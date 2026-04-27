export module volkus:vkx.error;
import std;
import :vkx.exports;

export namespace Volkus::vkx
{
	[[noreturn]] 
	inline void Fatal(std::string_view msg, std::source_location loc = std::source_location::current()) noexcept
	{
		auto stacktrace = std::stacktrace::current(1);
		std::println(
			std::cerr,
			"Fatal error: {}\n"
			"Location: {}:{} in {}\n"
			"Stack trace:\n{}",
			msg,
			loc.file_name(),
			loc.line(),
			loc.function_name(),
			stacktrace
		);
		std::abort();
	}

	auto ToString(VkResult result) -> std::string
	{
		switch (result)
		{
			case VK_SUCCESS: return "Success";
			case VK_NOT_READY: return "Not ready";
			case VK_TIMEOUT: return "Timeout";
			case VK_EVENT_SET: return "Event set";
			case VK_EVENT_RESET: return "Event reset";
			case VK_INCOMPLETE: return "Incomplete";
			case VK_ERROR_OUT_OF_HOST_MEMORY: return "Out of host memory";
			case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "Out of device memory";
			case VK_ERROR_INITIALIZATION_FAILED: return "Initialization failed";
			case VK_ERROR_DEVICE_LOST: return "Device lost";
			case VK_ERROR_MEMORY_MAP_FAILED: return "Memory map failed";
			case VK_ERROR_LAYER_NOT_PRESENT: return "Layer not present";
			case VK_ERROR_EXTENSION_NOT_PRESENT: return "Extension not present";
			case VK_ERROR_FEATURE_NOT_PRESENT: return "Feature not present";
			case VK_ERROR_INCOMPATIBLE_DRIVER: return "Incompatible driver";
			case VK_ERROR_TOO_MANY_OBJECTS: return "Too many objects";
			case VK_ERROR_FORMAT_NOT_SUPPORTED: return "Format not supported";
			case VK_ERROR_FRAGMENTED_POOL: return "Fragmented pool";
			case VK_ERROR_OUT_OF_POOL_MEMORY: return "Out of pool memory";
			case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "Invalid external handle";
			default: return std::format("Unknown error code: {}", static_cast<int>(result));
		}
	}

	struct Result
	{
		constexpr Result(VkResult code) noexcept : Code(code) {}
		explicit constexpr operator bool() const noexcept { return Code == VK_SUCCESS; }
		constexpr auto Get() const noexcept -> VkResult { return Code; }
		auto ToString() const -> std::string { return ::Volkus::vkx::ToString(Code); }
		VkResult Code{};
		constexpr operator VkResult() const noexcept { return Code; }
	};

	class VolkusError : public std::runtime_error
	{
	public:
		VolkusError(
			std::string_view message,
			const std::source_location& loc = std::source_location::current()
		) : std::runtime_error(Format(message, loc))
		{}

		static auto Format(
			std::string_view message,
			const std::source_location& location,
			const std::stacktrace& stacktrace = std::stacktrace::current(1)
		) -> std::string
		{
			return std::format(
				"Volkus error: {}\n"
				"Location: {}:{} in {}\n"
				"Stack trace:\n{}",
				message,
				location.file_name(),
				location.line(),
				location.function_name(),
				stacktrace
			);
		}
	};

	class VulkanError : public std::runtime_error
	{
	public:
		explicit VulkanError(
			VkResult result, 
			std::string_view message, 
			const std::source_location& loc = std::source_location::current()
		) : std::runtime_error(Format(result, message, loc)) 
		{}

		static auto Format(
			VkResult result, 
			std::string_view message, 
			const std::source_location& location,
			const std::stacktrace& stacktrace = std::stacktrace::current(1)
		) -> std::string
		{
			return std::format(
				"Vulkan error: {}\n"
				"Message: {}\n"
				"Location: {}:{} in {}\n"
				"Stack trace:\n{}",
				ToString(result),
				message,
				location.file_name(),
				location.line(),
				location.function_name(),
				stacktrace
			);
		}
	};
}