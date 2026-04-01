# graphics

High-level Vulkan initialization and rendering orchestration for the `vulkangfx`
module.

## Dependency rules

- `:graphics.*` partitions may depend on any composite or ground-level
  partition (e.g. `:vulkan`, `:glfw`, `:gsl`, `:win32`, `:stlhelpers`, `:error`,
  `:logging`, `:glm`, `:util`).
- This subsystem sits at **Layer 2** in the dependency hierarchy, above the
  composite partitions it consumes.

## Partitions

| Partition               | Layer    | Purpose                                                    |
|-------------------------|----------|------------------------------------------------------------|
| `:graphics`             | aggregator | Re-exports all `:graphics.*` sub-partitions              |
| `:graphics.vertex`      | internal | `Graphics::Vertex` — vertex structure with position and color (depends on `:vulkan`, `:glm`, `:util`) |
| `:graphics.corevulkan`  | internal | `Graphics::CoreVulkan` — chained Vulkan initialization (instance, device, swap chain, pipeline, commands, descriptors) |
