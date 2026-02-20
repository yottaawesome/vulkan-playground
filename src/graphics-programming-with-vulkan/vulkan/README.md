# vulkan

[Vulkan](https://www.vulkan.org/) API wrappers for the `vulkangfx` module.
Provides raw symbol re-exports, error handling, RAII smart pointer aliases, and
instance creation utilities.

## Dependency rules

- `:vulkan.exports` is **ground-level** and must not depend on any other
  `vulkangfx` partitions.
- Other `:vulkan.*` partitions may depend on `:vulkan.exports`, `:raii`, and
  sibling `:vulkan.*` partitions.
- Partitions in this subsystem must not depend on partitions from other
  subsystems (e.g. `:glfw.*`, `:win32.*`).

## Partitions

| Partition            | Layer        | Purpose                                                        |
|----------------------|--------------|----------------------------------------------------------------|
| `:vulkan`            | aggregator   | Re-exports all `:vulkan.*` sub-partitions                      |
| `:vulkan.exports`    | ground-level | Raw Vulkan symbol re-exports in `vkr` namespace, `MakeVersion`, `Versions` enum |
| `:vulkan.error`      | internal     | `Vulkan::Result`, `Vulkan::VulkanError`                        |
| `:vulkan.raii`       | internal     | `Vulkan::VkInstanceUniquePtr`                                  |
| `:vulkan.instance`   | internal     | `Vulkan::Instance::Factory`, `AppInfo`, `InstanceInfo`, `EnumerateSupportedExtensions` |
