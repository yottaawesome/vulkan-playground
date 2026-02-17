export module gfxwithvulkancourse;
import  std;
export import :exports;

template <auto VDeleteFn>
struct Deleter
{
	static constexpr void operator()(auto&& args) noexcept
	{
		VDeleteFn(std::forward<decltype(args)>(args));
	}
};

template <typename T, auto VDeleteFn>
using DirectUniquePtr = std::unique_ptr<T, Deleter<VDeleteFn>>;
template <typename T, auto VDeleteFn>
using IndirectUniquePtr = std::unique_ptr<std::remove_pointer_t<T>, Deleter<VDeleteFn>>;

using UniqueGlfwWindow = DirectUniquePtr<glfw::GLFWwindow, glfw::glfwDestroyWindow>;

struct GlfwWindowFactory
{
	std::string Title = "Vulkan";
	std::uint32_t ClientApi = glfw::NoApi;
	bool Resizable = false;
	std::uint32_t Width = 800;
	std::uint32_t Height = 600;

	auto Create(this const GlfwWindowFactory& self) -> UniqueGlfwWindow
	{
		glfw::glfwInit();
		glfw::glfwWindowHint(glfw::ClientApi, self.ClientApi);
		glfw::glfwWindowHint(glfw::Resizable, self.Resizable);
		auto window = glfw::glfwCreateWindow(self.Width, self.Height, self.Title.c_str(), nullptr, nullptr);
		if (not window)
			throw std::runtime_error("Failed to create GLFW window");
		return UniqueGlfwWindow{ window };
	}

	auto operator()(this const GlfwWindowFactory& self) -> UniqueGlfwWindow
	{
		return self.Create();
	}
};

struct GlfwContext
{
	GlfwContext()
	{
		glfw::glfwInit();
	}
	~GlfwContext()
	{
		glfw::glfwTerminate();
	}
};

export extern "C++" auto wWinMain(
    Win32::HINSTANCE, 
    Win32::HINSTANCE, 
    Win32::LPWSTR, 
    Win32::UINT
) -> int
{
	auto context = GlfwContext{};
	auto window = UniqueGlfwWindow{ GlfwWindowFactory{}.Create() };
		
	while (not glfw::glfwWindowShouldClose(window.get()))
	{
		glfw::glfwPollEvents();
	}

    return 0;
}
