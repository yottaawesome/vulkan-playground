# vulkangfx — Architecture

## Overview

`vulkangfx` is a C++23 module-based project for Vulkan graphics programming. The
project is structured as a single named module (`vulkangfx`) composed of
partitions, each encapsulating a distinct subsystem.

## Module partition tree

```
vulkangfx                          primary module interface (vulkangfx.ixx)
│
├── :raii                           [ground-level] generic RAII utilities
│
├── :glm                            [ground-level] GLM library re-exports
│   └── :glm.exports
│
├── :gsl                            [ground-level] GSL library re-exports
│
├── :stlhelpers                     [ground-level, internal] STL helper utilities
│                                   (not exported from primary module)
│
├── :win32                          Win32 API re-exports and wrappers
│   ├── :win32.exports              [ground-level]
│   ├── :win32.error                depends on :win32.exports
│   ├── :win32.raii                 depends on :raii, :win32.exports
│   └── :win32.event                depends on :win32.exports, :win32.raii, :win32.error
│
├── :glfw                           GLFW wrappers
│   ├── :glfw.exports               [ground-level] raw GLFW symbol re-exports
│   ├── :glfw.error                 depends on :glfw.exports
│   ├── :glfw.raii                  depends on :raii, :glfw.exports, :glfw.error
│   ├── :glfw.window                depends on :glfw.exports, :glfw.raii, :glfw.error, :gsl
│   ├── :glfw.monitor               depends on :glfw.exports, :glfw.error
│   └── :glfw.functions             depends on :glfw.exports, :gsl
│
├── :vulkan                         Vulkan API wrappers
│   ├── :vulkan.exports             [ground-level] raw Vulkan symbol re-exports
│   ├── :vulkan.error               depends on :vulkan.exports
│   ├── :vulkan.raii                depends on :raii, :vulkan.exports
│   ├── :vulkan.surface             depends on :vulkan.raii, :vulkan.error, :vulkan.exports
│   ├── :vulkan.physicaldevice      depends on :vulkan.exports, :vulkan.error
│   └── :vulkan.instance            depends on :vulkan.exports, :vulkan.error, :vulkan.raii,
│                                              :vulkan.surface, :win32
│
└── :graphics                       High-level graphics orchestration
    └── :graphics.corevulkan        depends on :vulkan, :glfw, :gsl, :win32
```

`main.cpp` is the application entry point and imports the `vulkangfx` module.

## Dependency layers

The partitions are organized into dependency layers. Lower layers must not
depend on higher layers.

```
Layer 4 — Application         main.cpp
                                │
Layer 3 — Primary module       vulkangfx
                                │
Layer 2 — High-level           :graphics
                                │
Layer 1 — Subsystem composites :glfw, :win32, :vulkan
                                │
Layer 0 — Ground-level         :raii, :glm, :gsl, :stlhelpers,
                               :win32.exports, :glfw.exports, :vulkan.exports
```

### Ground-level partitions

Ground-level partitions either wrap an external library or provide foundational
utilities. They must not depend on any other `vulkangfx` partitions (only on
`std` and the external libraries they wrap). Other partitions may freely depend
on them.

`:stlhelpers` is ground-level but is **not** exported from the primary module
interface. It is available as an internal implementation partition.

### Cross-subsystem dependencies

Most sub-partitions only depend on siblings within their own subsystem and on
ground-level partitions. The following partitions additionally depend on
partitions from other subsystems:

| Partition           | Cross-subsystem dependency | Reason |
|---------------------|---------------------------|--------|
| `:vulkan.instance`  | `:win32`                  | Win32 surface creation during instance setup |
| `:glfw.window`      | `:gsl`                    | `gsl::not_null` for pointer contracts |
| `:glfw.functions`   | `:gsl`                    | `gsl::not_null` for pointer contracts |

These cross-subsystem edges mean `:vulkan` and `:glfw` are not fully
self-contained — they sit at Layer 1 but have narrow, well-motivated reach-downs
into ground-level partitions from other subsystems.

### Composite partitions

Composite partitions (e.g. `:glfw`, `:win32`, `:vulkan`) aggregate and re-export
their sub-partitions. Their sub-partitions may depend on ground-level partitions
and on sibling sub-partitions within their own subsystem (plus the documented
cross-subsystem exceptions above).

## Subsystem directories

Each subsystem lives in its own directory with a consistent structure:

| Directory      | Subsystem      | Description |
|----------------|----------------|-------------|
| `raii/`        | `:raii`        | Generic RAII smart pointer utilities |
| `glm/`        | `:glm`         | GLM math library re-exports |
| `gsl/`        | `:gsl`         | GSL library re-exports |
| `stlhelpers/`  | `:stlhelpers`  | STL helpers — `Collection`, pipe adaptors, `NullMutex`, `ToVector` |
| `win32/`       | `:win32`       | Win32 API types, error handling, RAII wrappers, events |
| `glfw/`        | `:glfw`        | GLFW window/monitor management, error handling, RAII wrappers |
| `vulkan/`      | `:vulkan`      | Vulkan API types, instance/surface/physical-device management, error handling, RAII wrappers |
| `graphics/`    | `:graphics`    | High-level Vulkan initialization and rendering orchestration |

## Conventions

- **`.exports` partitions** contain raw `using` declarations to re-export C/C++
  library symbols into namespaced module interfaces. They reside in the global
  module fragment and must remain ground-level.
- **`.raii` partitions** provide RAII wrappers (smart pointer aliases) for
  handles/resources from their respective subsystem.
- **`.error` partitions** define error types for their subsystem.
- **Aggregator partitions** (e.g. `:glfw`, `:win32`) use `export import` to
  re-export all sub-partitions as a single import target.
