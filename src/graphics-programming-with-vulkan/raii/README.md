# raii

Generic RAII smart pointer utilities for the `vulkangfx` module.

## Dependency rules

This is a **ground-level** partition. It must not depend on any other `vulkangfx`
partitions. Other partitions may depend on `:raii`.

## Partitions

| Partition | Purpose |
|-----------|---------|
| `:raii`   | `Deleter`, `DirectUniquePtr`, and `IndirectUniquePtr` templates |
