export module vulkangfx:glfw.functions;
import std;
import :glfw.exports;
import :gsl;

export namespace glfw 
{
	auto GetRequiredVulkanExtensions() noexcept -> std::span<gsl::czstring>
	{
		auto extensionCount = std::uint32_t{};
		auto extensions = glfw::glfwGetRequiredInstanceExtensions(&extensionCount);
		return { extensions, extensionCount };
	}
}
