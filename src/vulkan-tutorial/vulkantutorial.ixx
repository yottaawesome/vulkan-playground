export module vulkantutorial;
import std;
export import :mainapp;

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
