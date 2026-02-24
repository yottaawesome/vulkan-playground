# stlhelpers

STL helper utilities for the `vulkangfx` module. Provides a `Collection`
wrapper with pipe-style adaptors, a `NullMutex`, and span-to-vector conversion.

> **Note:** This partition is **not** exported from the primary module interface
> (`vulkangfx.ixx`). It is available as an internal implementation partition
> via `import :stlhelpers;`.

## Dependency rules

This is a **ground-level** partition. It must not depend on any other `vulkangfx`
partitions — only on `std`.

## Partitions

| Partition      | Purpose |
|----------------|---------|
| `:stlhelpers`  | `StlHelpers::Collection`, `ToVector`, `NullMutex`, pipe adaptors (`filter`, `transform`, `collect`) |

## Key types

- **`Collection<T>`** — ranges-compatible wrapper with `filter`, `transform`,
  `erase_if`, `any_of`, `none_of`, `all_of`, `count_if`, and `size`. Supports
  `operator|` with both custom `FilterClosure`/`TransformClosure`/`CollectClosure`
  adaptors and standard `std::ranges::views` pipes.
- **`NullMutex`** — no-op mutex satisfying the Lockable concept, useful as a
  template policy for single-threaded contexts.
- **`ToVector`** — converts a `std::span<T>` to `std::vector<T>`.
