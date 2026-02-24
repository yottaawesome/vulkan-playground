# vulkan

[Vulkan](https://www.vulkan.org/) API wrappers for the `vulkangfx` module.
Provides raw symbol re-exports, error handling, RAII smart pointer aliases,
surface management, physical device enumeration, and instance creation utilities.

## Dependency rules

- `:vulkan.exports` is **ground-level** and must not depend on any other
  `vulkangfx` partitions.
- Other `:vulkan.*` partitions may depend on `:vulkan.exports`, `:raii`, and
  sibling `:vulkan.*` partitions.
- **Exception:** `:vulkan.instance` additionally depends on `:win32` (for Win32
  surface creation during instance setup). This is the only cross-subsystem
  dependency in this subsystem.

## Partitions

| Partition                | Layer        | Purpose                                                        |
|--------------------------|--------------|----------------------------------------------------------------|
| `:vulkan`                | aggregator   | Re-exports all `:vulkan.*` sub-partitions                      |
| `:vulkan.exports`        | ground-level | Raw Vulkan symbol re-exports in `vkr` namespace, `MakeVersion`, `Versions` enum, `Extensions`, `Layers`, `DebugUtilsMessageType` namespaces |
| `:vulkan.error`          | internal     | `Vulkan::Result`, `Vulkan::VulkanError`                        |
| `:vulkan.raii`           | internal     | `Vulkan::VkInstanceUniquePtr`                                  |
| `:vulkan.surface`        | internal     | `Vulkan::Surface` — movable, non-copyable Vulkan surface wrapper |
| `:vulkan.physicaldevice` | internal     | `Vulkan::PhysicalDevice`, `PhysicalDevieList`, `LogicalDevice` |
| `:vulkan.instance`       | internal     | `Vulkan::Instance::Factory`, `AppInfo`, `InstanceInfo`, `EnumerateSupportedExtensions` (depends on `:win32`) |
