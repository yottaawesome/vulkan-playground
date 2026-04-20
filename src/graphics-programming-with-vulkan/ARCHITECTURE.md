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
├── :error                          [ground-level] error types with source location
│
├── :string                         [ground-level] compile-time string utilities
│
├── :util                           [ground-level] miscellaneous utilities
│
├── :stlhelpers                     [ground-level] STL helper utilities
│   └── :stlhelpers.collection
│
├── :stb                            [ground-level] stb_image library re-exports
│   └── :stb.exports
│
├── :file                           file I/O utilities
│   ├── :file.file                  depends on :error
│   └── :file.functions             depends on :error
│
├── :win32                          Win32 API re-exports and wrappers
│   ├── :win32.exports              [ground-level]
│   ├── :win32.error                depends on :win32.exports
│   ├── :win32.raii                 depends on :raii, :win32.exports
│   └── :win32.event                depends on :win32.exports, :win32.raii, :win32.error
│
├── :logging                        logging utilities
│                                   depends on :win32, :error, :string, :file
│
├── :glfw                           GLFW wrappers
│   ├── :glfw.exports               [ground-level] raw GLFW symbol re-exports
│   ├── :glfw.error                 depends on :glfw.exports
│   ├── :glfw.raii                  depends on :raii, :glfw.exports, :glfw.error
│   ├── :glfw.window                depends on :error, :gsl, :glfw.exports, :glfw.raii,
│   │                                          :glfw.error
│   ├── :glfw.monitor               depends on :glfw.exports, :glfw.error
│   └── :glfw.functions             depends on :glfw.exports, :gsl
│
├── :vulkan                         Vulkan API wrappers
│   ├── :vulkan.exports             [ground-level] raw Vulkan symbol re-exports
│   ├── :vulkan.error               depends on :vulkan.exports
│   ├── :vulkan.formatters          depends on :vulkan.exports
│   ├── :vulkan.raii                depends on :raii, :vulkan.exports
│   ├── :vulkan.instance            depends on :win32, :vulkan.exports, :vulkan.error,
│   │                                          :vulkan.raii
│   ├── :vulkan.surface             depends on :win32, :error, :vulkan.raii,
│   │                                          :vulkan.error, :vulkan.exports
│   ├── :vulkan.physicaldevice      depends on :raii, :stlhelpers, :vulkan.exports,
│   │                                          :vulkan.error, :vulkan.formatters
│   ├── :vulkan.device              depends on :raii, :vulkan.exports, :vulkan.error
│   ├── :vulkan.devicequeue         depends on :error, :vulkan.exports, :vulkan.error
│   ├── :vulkan.swapchain           depends on :error, :vulkan.exports, :vulkan.error
│   ├── :vulkan.imageview           depends on :error, :vulkan.exports, :vulkan.error
│   ├── :vulkan.shaders             depends on :error, :file, :stlhelpers,
│   │                                          :vulkan.exports, :vulkan.error
│   ├── :vulkan.pipeline            depends on :raii, :vulkan.exports, :vulkan.error,
│   │                                          :vulkan.raii
│   ├── :vulkan.commands            depends on :error, :vulkan.error, :vulkan.exports
│   ├── :vulkan.buffer              depends on :error, :vulkan.exports, :vulkan.error
│   ├── :vulkan.sync                depends on :error, :vulkan.exports, :vulkan.error
│   ├── :vulkan.descriptors         depends on :error, :vulkan.exports, :vulkan.error
│   ├── :vulkan.uniformtransformations  depends on :glm
│   ├── :vulkan.texture             depends on :vulkan.exports, :vulkan.error,
│   │                                          :vulkan.buffer, :vulkan.imageview, :glm
│   └── :vulkan.texturesampler      depends on :error, :vulkan.exports, :vulkan.error
│
└── :graphics                       High-level graphics orchestration
    ├── :graphics.vertex            depends on :vulkan, :glm, :util
    └── :graphics.corevulkan        depends on :vulkan, :glfw, :gsl, :win32,
                                               :stlhelpers, :error, :logging,
                                               :graphics.vertex, :stb
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
Layer 2 — High-level           :graphics, :logging
                                │
Layer 1 — Subsystem composites :glfw, :win32, :vulkan, :file
                                │
Layer 0 — Ground-level         :raii, :glm, :gsl, :stlhelpers, :error, :string,
                               :util, :stb, :win32.exports, :glfw.exports,
                               :vulkan.exports
```

### Ground-level partitions

Ground-level partitions either wrap an external library or provide foundational
utilities. They must not depend on any other `vulkangfx` partitions (only on
`std` and the external libraries they wrap). Other partitions may freely depend
on them.

### Cross-subsystem dependencies

Most sub-partitions only depend on siblings within their own subsystem and on
ground-level partitions. Ground-level partitions such as `:raii`, `:error`,
`:gsl`, and `:stlhelpers` are widely used across subsystems and are not listed
individually below. The following table highlights the more notable
cross-subsystem dependencies:

| Partition                        | Cross-subsystem dependency | Reason |
|----------------------------------|---------------------------|--------|
| `:vulkan.instance`               | `:win32`                  | Win32 surface creation during instance setup |
| `:vulkan.surface`                | `:win32`                  | Win32 surface creation |
| `:vulkan.shaders`                | `:file`                   | Shader file loading |
| `:vulkan.uniformtransformations` | `:glm`                    | Matrix types for uniform buffers |
| `:vulkan.texture`                | `:glm`                    | `glm::ivec2` for image dimensions |
| `:glfw.window`                   | `:gsl`                    | `gsl::not_null` for pointer contracts |
| `:glfw.functions`                | `:gsl`                    | `gsl::not_null` for pointer contracts |
| `:graphics.corevulkan`           | `:logging`                | Initialization logging |
| `:graphics.corevulkan`           | `:stb`                    | Image loading for textures |

These cross-subsystem edges mean `:vulkan`, `:glfw`, and `:graphics` are not
fully self-contained — they sit at Layer 1–2 but have narrow, well-motivated
reach-downs into ground-level and utility partitions from other subsystems.

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
| `error/`       | `:error`       | Error types with source location tracking (`RuntimeError`, `IndexOutOfRangeError`) |
| `string/`      | `:string`      | Compile-time string utilities (`FixedString`) |
| `util/`        | `:util`        | Miscellaneous utilities (`OffsetOf`) |
| `stlhelpers/`  | `:stlhelpers`  | STL helpers — `Collection`, pipe adaptors, `NullMutex`, `ToVector` |
| `file/`        | `:file`        | File I/O utilities (`File`, `ReadFileBytes`) |
| `win32/`       | `:win32`       | Win32 API types, error handling, RAII wrappers, events |
| `logging/`     | `:logging`     | Named logger with console and file output (`Logger`) |
| `glfw/`        | `:glfw`        | GLFW window/monitor management, error handling, RAII wrappers |
| `stb/`         | `:stb`         | stb_image library re-exports for image loading |
| `vulkan/`      | `:vulkan`      | Vulkan API wrappers — instance, device, swapchain, pipeline, commands, buffers, synchronization, descriptors, shaders, textures, samplers |
| `graphics/`    | `:graphics`    | High-level Vulkan initialization and rendering orchestration |
| `shaders/`     | —              | GLSL shader sources and compiled SPIR-V binaries |

## Conventions

- **`.exports` partitions** contain raw `using` declarations to re-export C/C++
  library symbols into namespaced module interfaces. They reside in the global
  module fragment and must remain ground-level.
- **`.raii` partitions** provide RAII wrappers (smart pointer aliases) for
  handles/resources from their respective subsystem.
- **`.error` partitions** define error types for their subsystem.
- **Aggregator partitions** (e.g. `:glfw`, `:win32`) use `export import` to
  re-export all sub-partitions as a single import target.
