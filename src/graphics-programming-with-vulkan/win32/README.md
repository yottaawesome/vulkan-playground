# win32

Win32 API re-exports and wrappers for the `vulkangfx` module.

## Dependency rules

- `:win32.exports` is **ground-level** and must not depend on any other
  `vulkangfx` partitions.
- Other `:win32.*` partitions may depend on `:win32.exports`, `:raii`, and
  sibling `:win32.*` partitions.
- Partitions in this subsystem must not depend on partitions from other
  subsystems (e.g. `:glfw.*`, `:glm.*`).

## Partitions

| Partition         | Layer        | Purpose                                       |
|-------------------|--------------|-----------------------------------------------|
| `:win32`          | aggregator   | Re-exports all `:win32.*` sub-partitions      |
| `:win32.exports`  | ground-level | Raw Win32 symbol re-exports and constants     |
| `:win32.error`    | internal     | `Win32::Error`, `ErrorCodeToAnsi/Unicode`     |
| `:win32.raii`     | internal     | `HandleUniquePtr`, `HInstanceUniquePtr`       |
| `:win32.event`    | internal     | `CreateEventFactory` for Win32 event objects  |
