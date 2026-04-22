# Graphics Programming with Vulkan and C++ coursework

## Introduction

My coursework for [Graphics Programming with Vulkan and C++](https://www.udemy.com/course/graphics-programming-with-vulkan-and-cpp).

## Changes

I've actually not followed the course that closely, as I have introduced my own structure, classes and other modifications. The course material is rather dated due to its reliance on Vulkan 1.0 and the old renderpass/framebuffer functionality, so I've opted to use Vulkan 1.3+'s dynamic rendering instead.

## Warning

The course is actually quite dated; it's very much rooted in Vulkan 1.0 and the old renderpass/framebuffer functionality, so I would not recommend it. If you want to learn Vulkan, I would recommend looking for more up-to-date resources that cover Vulkan 1.3+ -- [How to Vulkan in 2026](https://www.howtovulkan.com) is a good start.

## Notes

### High-level Vulkan components

- **Instance**: Represents the connection between the application and the Vulkan library. It is used to query for available physical devices and create logical devices.
- **Physical Device**: Represents a physical GPU in the system. It is used to query for supported features and properties, and to create logical devices. Created from an instance.
- **Logical Device**: Represents a logical connection to a physical device. It is used to create resources such as buffers, images, and command pools. Created from a physical device.
- **Queue**: Represents a queue of commands that can be submitted to the GPU. It is used to execute command buffers and synchronize operations. Created from a logical device.
- **Command Pool**: Represents a pool of memory that can be used to allocate command buffers. It is used to manage the memory for command buffers and optimize their allocation and deallocation. Created from a logical device.
- **Command Buffer**: Represents an asynchronous sequence of commands that can be recorded and submitted to a queue for execution. It is used to perform operations such as drawing, copying, and synchronization. Created from a command pool.
- **Pipeline**: Represents a collection of state and resources that define how commands are executed on the GPU. It is used to specify the shaders, vertex input, rasterization, and other stages of the graphics pipeline. Created from a logical device.
- **Descriptor Set**: Represents a collection of resources (buffers, images, samplers) that can be bound to a pipeline for use in shaders. It is used to provide data to shaders during rendering. Created from a logical device.
- **Surface**: Represents a platform-specific window or display that can be used for presenting rendered images. It is used to manage the presentation of rendered images to the screen. Created from an instance and a windowing system (e.g. GLFW).
- **Swapchain**: Represents a collection of images that are presented to the screen. It is used to manage the presentation of rendered images to the display. Created from a logical device and a surface.
- **Memory buffers**: Represents memory allocated on the GPU for storing data such as vertex buffers, index buffers, and uniform buffers. It is used to manage the memory for resources and optimize their usage. Created from a logical device.