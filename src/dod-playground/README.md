# dod-playground

A tiny, dependency-free C++23 project for studying **data-oriented design** without
the ceremony of a real graphics API. No Vulkan, no GLFW, no glm — just plain
C++ modules, `std::vector`, and a stopwatch.

The point is to isolate the *ideas* of DOD (data layout, batched work, sorting
for cache/bind locality, frame graphs) from the ~90% of GPU code that is
API ceremony. Read the lesson files; they're short.

## Build

Open `dod-playground.slnx` in Visual Studio 2022+ (uses `stdcpplatest` + `BuildStlModules`)
and build the `x64` configuration of your choice.

Prefer **Release** for Lesson 1 — Debug bounds-checks on `std::vector::operator[]`
dominate the measurement and hide the real cache effect.

## Run

```
dod-playground.exe              # runs all lessons
dod-playground.exe 1            # runs lesson 1 only
dod-playground.exe --help
```

## Lessons

### 1. `dod.lesson01_aos_vs_soa.ixx` — Data layout and the cache

Frustum-cull 200k entities. One version uses `std::vector<EntityAoS>` where hot
bounds fields are interleaved with cold metadata (debug names, script handles,
transform matrix); the other uses SoA with hot fields in their own packed
arrays. Measures and prints min/median/mean wall time per pass.

**Ask yourself:** why does the AoS version stall even though `boundsCenter` and
`boundsRadius` are the first fields in the struct?

### 2. `dod.lesson02_sort_keys.ixx` — Order is data

Submits 10,000 random draw calls to a `FakeGpu` that counts state changes
(a proxy for real bind cost). Does it unsorted, then sorted by a packed
`[pipeline | material | depth]` 64-bit key. Prints the ratio.

**Ask yourself:** in which bit positions should `depth` go for transparent
draws (back-to-front) vs opaque draws (front-to-back)?

### 3. `dod.lesson03_indirect.ixx` — Cull → compact → single submit

50k scene entries in SoA. A cull pass reads the SoA and writes a compact
`std::vector<IndirectCmd>`. The "draw" stage is a single tight loop over that
contiguous output — exactly the shape of GPU-driven rendering with
`vkCmdDrawIndexedIndirect` / `ExecuteIndirect`.

**Ask yourself:** if `BuildIndirectBuffer` were a compute shader, what would
the `outCmds.push_back` line become?

### 4. `dod.lesson04_framegraph.ixx` — Passes are data

Declares 5 render passes as plain structs with read/write sets and a lambda
body. A small scheduler topologically orders them and prints the barriers it
would insert between passes that share a resource. Then it "executes".

**Ask yourself:** what changes if you want the shadow pass on the async
compute queue? (Hint: add a `queue` field to `Pass`. That's it.)

### 5. `dod.lesson05_bda.ixx` — Buffer device address, modelled in C++

Models Vulkan 1.2's `VK_KHR_buffer_device_address` with plain pointers.
A `SceneData` struct holds pointers to transform / material / mesh tables;
the "vertex shader" dereferences those pointers just like GLSL would with
`GL_EXT_buffer_reference`. Compares bind counts against a descriptor-set-style
submission of the same draws.

**Ask yourself:** what is the C++ equivalent of `vkGetBufferDeviceAddress`?
(Answer: `std::vector::data()`. The shape of BDA is exactly "hand the shader
a pointer to an array"; the only thing Vulkan adds is making sure the
pointer is a GPU virtual address rather than a host one.)

## Where to go next

- **Mike Acton — "Data-Oriented Design and C++"** (CppCon 2014)
- **Stoyan Nikolov — "OOP Is Dead, Long Live Data-Oriented Design"** (CppCon 2018)
- **EnTT** (`github.com/skypjack/entt`) — production SoA / sparse-set ECS in C++
- **Bitsquid blog** by Niklas Gray — "Managing Decoupling", "Data-Oriented, Data-Driven Systems"

When these ideas click, re-read the sibling `graphics-programming-with-vulkan`
project — the DOD gaps (per-draw binds, per-object descriptor sets, hand-wired
barriers, no sort keys) will be much more visible than before.
