import std;
import vulkantutorial;

auto main(int argc, char* argv[]) -> int
try
{
    auto app = VulkanTutorial::App::MainApp{};
    app.Run();
    return 0;
}
catch (const std::exception& ex)
{
    std::println("Exception in main: {}", ex.what());
    return 1;
}
