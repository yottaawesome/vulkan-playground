export module vulkangfx:glfw.monitor;
import std;
import :glfw.exports;
import :glfw.error;

export namespace glfw
{
	class Monitor
	{
	public:
		explicit Monitor(
			GLFWmonitor* monitor = [] -> GLFWmonitor*
			{
				auto primary = glfw::glfwGetPrimaryMonitor();
				return primary ? primary : throw Error("Failed to get primary monitor.");
			}()
		) : monitor(monitor)
		{
			if (not monitor)
				throw std::runtime_error("Monitor pointer cannot be null.");
		}

		auto GetVideoMode(this const Monitor& self) -> const GLFWvidmode*
		{
			auto mode = static_cast<const GLFWvidmode*>(glfwGetVideoMode(self.monitor));
			if (not mode)
				throw Error("Failed to get video mode for the monitor.");
			return mode;
		}

		auto GetScreenDimensions(this const Monitor& self) -> std::pair<int, int>
		{
			auto mode = static_cast<const GLFWvidmode*>(self.GetVideoMode());
			return { mode->width, mode->height };
		}

		auto GetMonitorPosition(this const Monitor& self) -> std::pair<int, int>
		{
			int xPos, yPos;
			glfwGetMonitorPos(self.monitor, &xPos, &yPos);
			return { xPos, yPos };
		}

		struct WorkArea
		{
			int XPos = 0;
			int YPos = 0;
			int Width = 0;
			int Height = 0;
		};

		auto GetMonitorWorkArea(this const Monitor& self) -> WorkArea
		{
			int xPos, yPos, width, height;
			glfwGetMonitorWorkarea(self.monitor, &xPos, &yPos, &width, &height);
			return { xPos, yPos, width, height };
		}

		static auto GetAllMonitors() -> std::vector<Monitor>
		{
			int count = 0;
			auto monitors = std::span{ glfwGetMonitors(&count), static_cast<std::uint64_t>(std::max(count, 0)) };
			return monitors 
				| std::ranges::views::transform([](GLFWmonitor* monitor) { return Monitor(monitor); })
				| std::ranges::to<std::vector<Monitor>>();
		}

	private:
		GLFWmonitor* monitor = nullptr;
	};
}