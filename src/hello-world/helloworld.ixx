export module helloworld;
import std;
import vulkan;
export import :hellotriangleapplication;

extern "C" auto main() -> int
try 
{
    HelloTriangle::Application app;
    app.Run();
    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}
