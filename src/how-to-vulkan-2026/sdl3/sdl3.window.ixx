export module vulkan26:sdl3.window;
import std;
import :sdl3.error;
import :sdl3.exports;
import :glm;

export namespace sdl3
{
	class Window
	{
	public:
		~Window()
		{
			if (window)
			{
				SDL_DestroyWindow(window);
				window = nullptr;
			}
		}
		Window(std::string_view title, int w, int h, std::uint32_t flags = 0)
			: window(SDL_CreateWindow(title.data(), w, h, flags))
		{
			if (not window)
				throw Error::Error{};
		}

		// No copy semantics
		Window(Window const&) = delete;
		Window& operator=(Window const&) = delete;

		// Move semantics
		Window(Window&& other) noexcept
			: window(std::exchange(other.window, nullptr))
		{}
		auto operator=(this Window& self, Window&& other) noexcept -> Window&
		{
			if (&self == &other)
				return self;
			if (self.window)
				SDL_DestroyWindow(self.window);
			self.window = std::exchange(other.window, nullptr);
			return self;
		}

		auto GetWindowSize(this const auto& self) noexcept -> glm::ivec2
		{
			auto w = int{};
			auto h = int{};
			SDL_GetWindowSize(self.window, &w, &h);
			return { w, h };
		}

		constexpr auto Get(this const auto& self) noexcept -> SDL_Window*
		{
			return self.window;
		}

		auto CreateSurface(this const auto& self, vk::VkInstance instance) -> vk::VkSurfaceKHR
		{
			auto surface = vk::VkSurfaceKHR{};
			if (not SDL_Vulkan_CreateSurface(self.window, instance, nullptr, &surface))
				throw Error::Error{};
			return surface;
		}
	private:
		SDL_Window* window = nullptr;
	};
}