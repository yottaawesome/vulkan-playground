# vulkan

[Vulkan](https://www.vulkan.org/) API wrappers for the `vulkangfx` module.
Provides raw symbol re-exports, error handling, RAII smart pointer aliases,
instance creation, device management, swapchain, pipeline, command buffers,
buffer management, synchronization primitives, descriptor sets, shader loading,
and image views.

## Dependency rules

- `:vulkan.exports` and `:vulkan.formatters` are **ground-level** and must not
  depend on any other `vulkangfx` partitions.
- Other `:vulkan.*` partitions may depend on `:vulkan.exports`, `:raii`, and
  sibling `:vulkan.*` partitions.
- Several partitions additionally depend on the shared `:error` partition for
  error handling.
- **Notable cross-subsystem dependencies:**
  - `:vulkan.instance` and `:vulkan.surface` depend on `:win32` (for Win32
    surface creation).
  - `:vulkan.shaders` depends on `:file` and `:stlhelpers` (for shader file
    loading).
  - `:vulkan.physicaldevice` depends on `:stlhelpers` (for collection
    utilities).
  - `:vulkan.uniformtransformations` depends on `:glm` (for matrix types).

## Partitions

| Partition                        | Layer        | Purpose                                                        |
|----------------------------------|--------------|----------------------------------------------------------------|
| `:vulkan`                        | aggregator   | Re-exports all `:vulkan.*` sub-partitions                      |
| `:vulkan.exports`                | ground-level | Raw Vulkan symbol re-exports in `vkr` namespace, `MakeVersion`, `MakeApiVersion`, `Versions` enum, `Extensions`, `Layers`, `DebugUtilsMessageType` namespaces |
| `:vulkan.error`                  | internal     | `Vulkan::Result`, `Vulkan::VulkanError`                        |
| `:vulkan.formatters`             | ground-level | `std::formatter` specializations for `VkResult`, `VkPhysicalDeviceType` |
| `:vulkan.raii`                   | internal     | `Vulkan::VkInstanceUniquePtr`                                  |
| `:vulkan.instance`               | internal     | `Vulkan::Instance::Factory`, `AppInfo`, `InstanceInfo`, `MainInstance`, `EnumerateSupportedExtensions`, `EnumerateInstanceLayers` (depends on `:win32`) |
| `:vulkan.surface`                | internal     | `Vulkan::Surface`, `SurfaceFactory`, `SurfaceUniquePtr` (depends on `:win32`, `:error`) |
| `:vulkan.physicaldevice`         | internal     | `Vulkan::PhysicalDevice`, `PhysicalDeviceList`, `DeviceQueueDetails` |
| `:vulkan.device`                 | internal     | `Vulkan::Device`, `VkDeviceUniquePtr`, `DeviceFactory`         |
| `:vulkan.devicequeue`            | internal     | `Vulkan::DeviceQueue`                                          |
| `:vulkan.swapchain`              | internal     | `Vulkan::Swapchain`, `SwapchainFactory`, `SwapchainCapabilities`, `SwapchainImages` |
| `:vulkan.imageview`              | internal     | `Vulkan::ImageView`, `ImageViewFactory`                        |
| `:vulkan.shaders`                | internal     | `Vulkan::ShaderModule`, `ShaderModuleFactory` (depends on `:file`, `:stlhelpers`) |
| `:vulkan.pipeline`               | internal     | `Vulkan::Pipeline`, `PipelineLayout`, `PipelineFactory`, `PipelineLayoutFactory` |
| `:vulkan.commands`               | internal     | `Vulkan::CommandBuffer`, `CommandPool`                         |
| `:vulkan.buffer`                 | internal     | `Vulkan::VertexBuffer<T>`, `IndexBuffer`, `BufferHandle`, `FindMemoryType` |
| `:vulkan.sync`                   | internal     | `Vulkan::Sync::TimelineSemaphore`, `BinarySemaphore`, `Fence`  |
| `:vulkan.descriptors`            | internal     | `Vulkan::DescriptorSetLayout`, `DescriptorPool`, `DescriptorSet` |
| `:vulkan.uniformtransformations` | internal     | `Vulkan::UniformTransformations` (depends on `:glm`)           |
