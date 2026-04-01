# glfw

[GLFW](https://www.glfw.org/) wrappers for the `vulkangfx` module. Provides
window management, monitor queries, RAII resource wrappers, and error handling.

## Dependency rules

- `:glfw.exports` is **ground-level** and must not depend on any other
  `vulkangfx` partitions.
- Other `:glfw.*` partitions may depend on `:glfw.exports`, `:raii`, and
  sibling `:glfw.*` partitions.
- **Exceptions:** `:glfw.window` additionally depends on `:gsl` and `:error`.
  `:glfw.functions` additionally depends on `:gsl`.

## Partitions

| Partition         | Layer        | Purpose                                          |
|-------------------|--------------|--------------------------------------------------|
| `:glfw`           | aggregator   | Re-exports all `:glfw.*` sub-partitions          |
| `:glfw.exports`   | ground-level | Raw GLFW symbol re-exports, `WindowHints`, `InitHints`, `VulkanSupport` |
| `:glfw.error`     | internal     | `glfw::Error`, `VulkanSupportError`, `SimpleError`, `SetErrorCallback`, `GetErrorCallback` |
| `:glfw.raii`      | internal     | `GlfwWindowUniquePtr`, `glfw::Context`           |
| `:glfw.window`    | internal     | `Window` with nested `Factory`, `ScreenCoordinates`, `PixelCoordinates`, `ScreenDimensions` (depends on `:gsl`, `:error`) |
| `:glfw.monitor`   | internal     | `Monitor` (primary, position, work area, video mode) |
| `:glfw.functions`  | internal     | Vulkan-related helpers (`GetRequiredVulkanExtensions`) (depends on `:gsl`) |

## Threading

GLFW requires that window creation, destruction, event processing, and most
window/monitor functions are called from the **main thread**. See the
[GLFW thread safety docs](https://www.glfw.org/docs/3.3/intro_guide.html#thread_safety)
for details on which functions may be called from other threads.
