# stb

Re-exports [stb_image](https://github.com/nothings/stb) types and functions as
a module partition of `vulkangfx`. Used for loading image data from memory for
texture creation.

## Dependency rules

This is a **ground-level** partition. It must not depend on any other `vulkangfx`
partitions. Other partitions may depend on `:stb`.

## Partitions

| Partition       | Purpose                                          |
|-----------------|--------------------------------------------------|
| `:stb`          | Aggregates and re-exports sub-partitions         |
| `:stb.exports`  | Raw stb_image symbol re-exports (`stbi_uc`, `stbi_load_from_memory`, `stbi_image_free`, channel constants) |
