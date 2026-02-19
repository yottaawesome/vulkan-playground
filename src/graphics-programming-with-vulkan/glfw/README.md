# glfw

[GLFW](https://www.glfw.org/) wrappers for the `vulkangfx` module. Provides
window management, monitor queries, RAII resource wrappers, and error handling.

## Dependency rules

- `:glfw.exports` is **ground-level** and must not depend on any other
  `vulkangfx` partitions.
- Other `:glfw.*` partitions may depend on `:glfw.exports`, `:raii`, and
  sibling `:glfw.*` partitions.
- Partitions in this subsystem must not depend on partitions from other
  subsystems (e.g. `:win32.*`, `:glm.*`).

## Partitions

| Partition         | Layer        | Purpose                                          |
|-------------------|--------------|--------------------------------------------------|
| `:glfw`           | aggregator   | Re-exports all `:glfw.*` sub-partitions          |
| `:glfw.exports`   | ground-level | Raw GLFW symbol re-exports, `WindowHints`, `InitHints` |
| `:glfw.error`     | internal     | `glfw::Error` with `glfwGetError` integration    |
| `:glfw.raii`      | internal     | `GlfwWindowUniquePtr`, `glfw::Context`           |
| `:glfw.window`    | internal     | `WindowFactory`, `Window`                        |
| `:glfw.monitor`   | internal     | `Monitor` (primary, position, work area, video mode) |

## Threading

GLFW requires that window creation, destruction, event processing, and most
window/monitor functions are called from the **main thread**. See the
[GLFW thread safety docs](https://www.glfw.org/docs/3.3/intro_guide.html#thread_safety)
for details on which functions may be called from other threads.
