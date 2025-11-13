module;

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>


export module main;
import std;

struct HelloTriangleApplication 
{
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan() {

    }

    void mainLoop() {

    }

    void cleanup() {

    }
};

extern "C" int main()
try 
{
    HelloTriangleApplication app;
    app.run();
    return 0;
}
catch (const std::exception& ex)
{
    std::println("{}", ex.what());
    return 1;
}
