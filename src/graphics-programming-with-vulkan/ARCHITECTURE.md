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
├── :win32                          [ground-level] Win32 API re-exports and wrappers
│   ├── :win32.exports
│   ├── :win32.error                depends on :win32.exports
│   ├── :win32.raii                 depends on :raii, :win32.exports
│   └── :win32.event                depends on :win32.exports, :win32.raii, :win32.error
│
├── :glfw                           GLFW wrappers
│   ├── :glfw.exports               [ground-level] raw GLFW symbol re-exports
│   ├── :glfw.error                 depends on :glfw.exports
│   ├── :glfw.raii                  depends on :raii, :glfw.exports, :glfw.error
│   ├── :glfw.window                depends on :glfw.exports, :glfw.raii, :glfw.error
│   ├── :glfw.monitor               depends on :glfw.exports, :glfw.error
│   └── :glfw.functions             depends on :glfw.exports, :gsl
│
├── :vulkan                         Vulkan API wrappers
│   ├── :vulkan.exports             [ground-level] raw Vulkan symbol re-exports
│   ├── :vulkan.error               depends on :vulkan.exports
│   ├── :vulkan.raii                depends on :raii, :vulkan.exports
│   └── :vulkan.instance            depends on :vulkan.exports, :vulkan.error, :vulkan.raii
│
└── :graphics                       High-level graphics orchestration
    └── :graphics.corevulkan        depends on :vulkan, :glfw, :gsl
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
Layer 1 — Composite partitions :glfw, :win32, :vulkan
                                │
Layer 0 — Ground-level         :raii, :glm, :gsl, :win32.exports, :glfw.exports, :vulkan.exports
```

### Ground-level partitions

Ground-level partitions either wrap an external library or provide foundational
utilities. They must not depend on any other `vulkangfx` partitions (only on
`std` and the external libraries they wrap). Other partitions may freely depend
on them.

### Composite partitions

Composite partitions (e.g. `:glfw`, `:win32`) aggregate and re-export their
sub-partitions. They may depend on ground-level partitions and on sibling
sub-partitions within their own subsystem.

## Subsystem directories

Each subsystem lives in its own directory with a consistent structure:

| Directory | Subsystem | Description |
|-----------|-----------|-------------|
| `raii/`   | `:raii`   | Generic RAII smart pointer utilities |
| `glm/`   | `:glm`   | GLM math library re-exports |
| `gsl/`   | `:gsl`   | GSL library re-exports |
| `win32/`  | `:win32`  | Win32 API types, error handling, RAII wrappers, events |
| `glfw/`   | `:glfw`   | GLFW window/monitor management, error handling, RAII wrappers |
| `vulkan/` | `:vulkan` | Vulkan API types, instance creation, error handling, RAII wrappers |
| `graphics/` | `:graphics` | High-level Vulkan initialization and rendering orchestration |

## Conventions

- **`.exports` partitions** contain raw `using` declarations to re-export C/C++
  library symbols into namespaced module interfaces. They reside in the global
  module fragment and must remain ground-level.
- **`.raii` partitions** provide RAII wrappers (smart pointer aliases) for
  handles/resources from their respective subsystem.
- **`.error` partitions** define error types for their subsystem.
- **Aggregator partitions** (e.g. `:glfw`, `:win32`) use `export import` to
  re-export all sub-partitions as a single import target.
