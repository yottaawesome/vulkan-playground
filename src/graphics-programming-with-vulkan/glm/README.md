# glm

Re-exports [GLM](https://github.com/g-truc/glm) types and functions as a module
partition of `vulkangfx`.

## Dependency rules

This is a **ground-level** partition. It must not depend on any other `vulkangfx`
partitions. Other partitions may depend on `:glm`.

## Partitions

| Partition       | Purpose                                          |
|-----------------|--------------------------------------------------|
| `:glm`          | Aggregates and re-exports sub-partitions                 |
| `:glm.exports`  | Raw GLM symbol re-exports (`vec2`–`vec4`, `mat2`–`mat4`, `ivec1`–`ivec4`, `rotate`, `radians`, `perspective`, `translate`, `lookAt`) |
