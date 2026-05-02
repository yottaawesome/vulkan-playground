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

## Crash course: SIMD, cache lines, and DOD

If the lessons below are the *practice*, this section is the *theory* — the three
ideas that make data-oriented design pay off on real hardware.

### 1. SIMD (Single Instruction, Multiple Data)

Modern CPUs have wide registers (128/256/512 bits) that operate on **multiple
values in one instruction**.

```
Scalar:  a[0] + b[0] = c[0]                                 (1 add per cycle)
SIMD:    [a0,a1,a2,a3] + [b0,b1,b2,b3] = [c0,c1,c2,c3]      (4 adds per cycle)
```

- **SSE** = 128-bit (4 floats), **AVX2** = 256-bit (8 floats), **AVX-512** = 512-bit (16 floats).
- Compilers auto-vectorize *if* data is contiguous, aligned, and loop bodies are branch-free.
- Intrinsics: `_mm256_add_ps`, `_mm_load_ps`, etc.

**Key constraint:** SIMD wants **homogeneous arrays of values**, not arrays of
fat structs.

### 2. Cache lines

RAM is slow (~100 ns); L1 cache is fast (~1 ns). The CPU never reads a single
byte — it reads a whole **cache line** (typically **64 bytes**) at a time.

```
[ ─── 64 bytes ─── ]   ← one fetch from memory
```

Implications:

- Reading 1 byte costs the same as reading 64 contiguous bytes.
- **Sequential access** ≈ free (the hardware prefetcher predicts it).
- **Random / pointer-chasing access** = stall city.
- **False sharing:** two threads writing to the same cache line bounce it
  between cores → tanks performance.

Rule of thumb: a cache miss to main memory is ~300–400 cycles. You can do *a
lot* of math in that time.

### 3. AoS vs SoA — the central DOD shift

**Array of Structs (OOP-typical):**

```cpp
struct Particle { vec3 pos; vec3 vel; vec3 color; float mass; bool alive; };
std::vector<Particle> particles;  // 40+ bytes per element
```

If you only need `pos` to update positions, you still drag `vel`, `color`,
`mass`, `alive` through cache. ~70% of bandwidth wasted.

**Struct of Arrays (DOD):**

```cpp
struct Particles {
    std::vector<float> px, py, pz;
    std::vector<float> vx, vy, vz;
    std::vector<bool>  alive;
};
```

Now `px[]` is a tight contiguous stream → perfect for the prefetcher and SIMD
(`_mm256_load_ps(&px[i])`). This is exactly what Lesson 1 demonstrates.

### 4. What data-oriented design actually is

DOD = **design around the data transformations the program performs**, not
around real-world "objects."

Core principles:

1. **Where there's one, there's many** — solve for the bulk case (process 10,000, not 1).
2. **Group by access pattern**, not by conceptual identity. Hot fields together, cold fields elsewhere.
3. **Linear arrays > pointers / trees** when iteration dominates.
4. **Sort / bucket** to make branches predictable (kills branch mispredicts).
5. **Indices, not pointers** — smaller, relocatable, cache-friendly (see Lesson 3).
6. **Separate hot / cold data** — `alive` flags in their own array means
   dead-particle skipping doesn't pollute cache.

### 5. How they compose

| Problem                  | DOD fix                          | Wins                            |
|--------------------------|----------------------------------|---------------------------------|
| Wasted cache bandwidth   | SoA layout                       | More useful bytes per line      |
| Slow scalar loops        | Contiguous arrays                | Auto-vectorization (SIMD)       |
| Branch mispredicts       | Sort by key first                | Straight-line code              |
| Pointer chasing          | Index handles into flat arrays   | Prefetcher-friendly             |
| False sharing            | Pad / partition per-thread data  | Scales with cores               |

### 6. Mental model

> The CPU is a **streaming machine** with attached scratchpads (caches). Feed
> it long, dense, predictable streams of homogeneous data and it runs near
> peak. Feed it pointer soup of heterogeneous objects and it spends 90% of its
> time waiting on memory.

OOP optimises for *programmer ergonomics*; DOD optimises for *the machine that
actually runs the code*. SIMD is the reward you get when you've already done
the DOD work — bad data layout makes SIMD impossible no matter how clever your
intrinsics are.

The lessons below walk through this progression: AoS↔SoA, sort keys,
indirection, frame graph, BDA.

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
