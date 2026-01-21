export module vulkantutorial;
export import std;
export import :libs;
export import :mainapp;
export import :error;
export import :util;
export import :formatters;

export auto main(int argc, char* argv[]) -> int
try
{
    VulkanTutorial::MainApp app;
    app.Run();
    return 0;
}
catch (const std::exception& ex)
{
    std::println("Exception in main: {}", ex.what());
    return 1;
}
