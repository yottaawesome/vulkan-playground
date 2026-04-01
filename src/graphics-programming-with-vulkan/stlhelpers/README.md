# stlhelpers

STL helper utilities for the `vulkangfx` module. Provides a `Collection`
wrapper with pipe-style adaptors, a `NullMutex`, and span-to-vector conversion.

## Dependency rules

This is a **ground-level** partition. It must not depend on any other `vulkangfx`
partitions — only on `std`.

## Partitions

| Partition               | Purpose |
|-------------------------|---------|
| `:stlhelpers`           | Aggregator — re-exports `:stlhelpers.collection` |
| `:stlhelpers.collection`| `StlHelpers::Collection`, `Vector`, `ToVector`, `NullMutex`, pipe adaptors (`filter`, `transform`, `collect`) |

## Key types

- **`Collection<T>`** — ranges-compatible wrapper with `filter`, `transform`,
  `flat_map`, `sorted`, `sorted_by`, `erase_if`, `any_of`, `none_of`, `all_of`,
  `count_if`, `find_if`, `contains`, `contains_if`, `fold_left`, `take`, `skip`,
  and `size`. Supports `operator|` with both custom
  `FilterClosure`/`TransformClosure`/`CollectClosure` adaptors and standard
  `std::ranges::views` pipes.
- **`Vector<T>`** — alias for `Collection<std::vector<T>>`.
- **`NullMutex`** — no-op mutex satisfying the Lockable concept, useful as a
  template policy for single-threaded contexts.
- **`ToVector`** — converts a `std::span<T>` to `std::vector<T>`.
