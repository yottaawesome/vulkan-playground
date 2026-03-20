# D3D12 / Vulkan Game Engine — Design Notes & Resources

A collection of architectural guidance, practical workflows, and learning resources for building a rendergraph-based 3D game engine using D3D12 or Vulkan.

---

## Table of Contents

1. [COM Interface Wrapper Design (D3D12)](#1-com-interface-wrapper-design-d3d12)
2. [Level Design Without a Custom Editor](#2-level-design-without-a-custom-editor)
3. [Using Maya Instead of Blender](#3-using-maya-instead-of-blender)
4. [Rendergraph Engine Scope Estimates](#4-rendergraph-engine-scope-estimates)
5. [Learning Resources for Rendergraph-Based Engines](#5-learning-resources-for-rendergraph-based-engines)

---

## 1. COM Interface Wrapper Design (D3D12)

### The Question

When building C++ wrapper objects around D3D12 COM interfaces (e.g., `ID3D12Resource`, `ID3D12PipelineState`), should the wrapper know how to construct the underlying COM object internally, or should it receive an already-constructed COM interface and delegate creation to a factory or free function?

### Recommendation: Inject the COM Interface from Outside

Wrappers should **receive** pre-constructed COM interfaces rather than constructing them internally. This follows a dependency-injection style pattern where creation logic is separated from ownership and usage.

#### Rationale

1. **Testability and flexibility** — Wrappers that construct their own COM objects are tightly coupled to a specific creation path. D3D12 object creation often depends on a `ID3D12Device`, descriptor sizes, heap properties, and other context. Baking that into the wrapper makes it rigid and hard to test or reuse.

2. **D3D12 creation is highly contextual** — Creating an `ID3D12Resource`, for example, varies significantly depending on whether it is a committed, placed, or reserved resource, which heap it targets, its initial resource state, optimized clear values, and so on. A wrapper should not need to encode all of these variations — it should simply own and expose the resulting object.

3. **Separation of concerns** — Wrappers become thin RAII handles: they own the `ComPtr<T>`, expose a clean domain-specific API over it, and release on destruction. Construction logic lives in a factory, builder, or free function that has the full context needed to call the appropriate `Create*` method on the device.

#### Example Pattern

```cpp
// Wrapper: thin RAII handle + domain API
class Texture {
public:
    explicit Texture(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_DESC desc)
        : resource_(std::move(resource)), desc_(desc) {}

    ID3D12Resource* Get() const { return resource_.Get(); }
    UINT GetWidth() const { return static_cast<UINT>(desc_.Width); }
    UINT GetHeight() const { return desc_.Height; }
    DXGI_FORMAT GetFormat() const { return desc_.Format; }
    // ... additional domain methods ...

private:
    ComPtr<ID3D12Resource> resource_;
    D3D12_RESOURCE_DESC desc_;
};

// Factory / free function: knows how to create the COM object
Texture CreateTexture2D(ID3D12Device* device, UINT width, UINT height, DXGI_FORMAT fmt) {
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.Format = fmt;
    desc.MipLevels = 1;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc = { 1, 0 };
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_HEAP_PROPERTIES heapProps = { D3D12_HEAP_TYPE_DEFAULT };

    ComPtr<ID3D12Resource> resource;
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&resource)
    ));

    return Texture(std::move(resource), desc);
}
```

**Summary**: The wrapper owns lifetime; the factory owns creation logic. This keeps both simple, composable, and independently evolvable.

---

## 2. Level Design Without a Custom Editor

### The Question

When building a small D3D12-based 3D game, how can level design be handled practically without creating a full-blown 3D engine and accompanying sophisticated level editor?

### Recommendation: Use Blender as Your Level Editor

Blender (or any established 3D tool) can serve as a fully capable level editor. Author levels as Blender scenes, export to **glTF 2.0** (.gltf/.glb), and load them in your engine with a lightweight parser.

#### The Data Pipeline

```
Blender scene  →  glTF export  →  Your loader  →  Game objects
```

#### How Each Concern Is Handled

| Concern | Approach |
|---|---|
| **Geometry & materials** | Loaded directly from glTF meshes and materials. |
| **Entity placement** | Each named object in Blender becomes a game entity. Use naming conventions like `enemy_grunt_01`, `light_point_03`, `trigger_zone_boss`. |
| **Metadata / game properties** | Blender's **custom properties** on objects export into glTF's `extras` JSON fields. Use these for game-specific data (e.g., `"type": "spawn_point"`, `"trigger": "door_open"`, `"health": 100`). |
| **Collision** | Either generate simple collision volumes from meshes at load time, or author low-poly collision hulls as hidden objects in Blender (e.g., prefix with `col_`). |

#### glTF Loader Libraries

Do not write a glTF parser from scratch. Use one of:

- **cgltf** — Single-header C library. Minimal, fast, easy to integrate.
- **tinygltf** — C++ library. Slightly heavier, handles image loading as well.

Both give you direct access to buffers, transforms, materials, and `extras` metadata with minimal friction.

#### Achieving Quick Iteration

- **Hot-reload**: Watch the `.glb` file for changes on disk. When the file is re-saved from Blender, reload the level automatically in your engine.
- **Config files alongside levels**: Use JSON or TOML files for gameplay tuning (enemy health, spawn rates, trigger conditions) so you don't need to round-trip through Blender for every parameter tweak.
- **Debug overlay**: A simple **Dear ImGui** layer to nudge object positions, tweak values, and inspect state at runtime is far cheaper to build than a real editor and covers most iteration needs.

#### When This Approach Breaks Down

This workflow works well for small-to-medium games. You will start to feel friction if you need:

- **Complex scripted event sequences** — Consider adding a lightweight scripting layer. Lua is a popular choice for its small footprint and easy C/C++ integration.
- **Procedural generation** — This is inherently code-driven; no editor helps here regardless.
- **Large teams editing simultaneously** — Requires proper asset pipelines and merge-friendly formats, but this is unlikely for a small project.

---

## 3. Using Maya Instead of Blender

### The Question

Can Maya be used in the same Blender-based level design workflow described above?

### Answer: Yes, With Minor Differences

Maya works for the same workflow. The core concepts (scene hierarchy as level layout, named objects as entities, custom attributes for metadata) are identical. The differences are mostly in export ergonomics and cost.

#### Comparison

| Concern | Blender | Maya |
|---|---|---|
| **Cost** | Free and open-source | Expensive (~$300/yr for Indie license) |
| **glTF support** | Native, excellent | Plugin-based (e.g., maya-glTF, Babylon exporter). Quality varies. |
| **Scripting** | Python, simple UI | Python and MEL — more powerful but heavier |
| **Custom properties UX** | Straightforward properties panel | "Add Attribute" dialog — more steps, same end result |
| **FBX export quality** | Decent | Industry-best (Autodesk owns the FBX format) |

#### Export Format Options from Maya

1. **glTF via plugin** — Works, but plugin quality and feature coverage can vary. Test your specific content types (materials, animations, custom attributes) early to avoid surprises.

2. **FBX → offline conversion to glTF** — More reliable, since Maya's FBX export is rock-solid. Use Meta's **FBX2glTF** tool or **Assimp** as an offline conversion step in your asset pipeline.

3. **FBX directly** — If you would rather skip glTF entirely, load FBX files straight into your engine using:
   - **ufbx** — Single-header C library, excellent quality and performance.
   - **Assimp** — Multi-format C++ library. Heavier, but supports dozens of formats.

#### Recommendation

If you already know Maya and are comfortable in it, use it. The workflow is essentially identical to the Blender-based one — the only extra friction is the export path being slightly less turnkey than Blender's native glTF support. If you are starting fresh with no existing preference, **Blender is the simpler choice** purely because of zero cost and first-class glTF support.

---

## 4. Rendergraph Engine Scope Estimates

### The Question

How many lines of code are typically required to build a minimal but functional rendergraph-based 3D engine in Vulkan or D3D12?

### Vulkan Estimate

| Component | Approx. Lines of Code |
|---|---|
| Instance, device, and swapchain setup | 1,500 – 2,500 |
| Render graph (pass declaration, dependency tracking, barrier insertion) | 2,000 – 4,000 |
| Resource management (images, buffers, descriptors, VMA integration) | 1,500 – 3,000 |
| Pipeline and shader management | 1,000 – 2,000 |
| Mesh loading and scene graph | 1,000 – 2,000 |
| Camera, transforms, basic math utilities | 500 – 1,000 |
| Main loop and frame orchestration | 500 – 1,000 |
| **Total** | **~8,000 – 16,000** |

### D3D12 Estimate

| Component | Approx. Lines of Code |
|---|---|
| Device, swapchain, and command queue setup | 800 – 1,500 |
| Render graph | 2,000 – 4,000 |
| Resource management (heaps, descriptors, upload/default buffers) | 1,200 – 2,500 |
| Pipeline, root signature, and shader management | 800 – 1,500 |
| Mesh loading and scene graph | 1,000 – 2,000 |
| Camera, transforms, basic math utilities | 500 – 1,000 |
| Main loop and frame orchestration | 400 – 800 |
| **Total** | **~6,500 – 13,000** |

### Why D3D12 Tends to Be Slightly Less Code

- No equivalent of Vulkan's verbose instance creation, layer enumeration, and extension negotiation.
- Swapchain setup is simpler — DXGI is more straightforward than `VkSurfaceKHR` + surface capabilities queries.
- Descriptor handling is different but not necessarily more code — root signatures are compact.
- No render pass / subpass ceremony — D3D12's barrier model maps more naturally to a rendergraph's resource-transition approach.

### The Rendergraph Subsystem Specifically

The render graph itself is roughly the same complexity in both APIs (~2,000–4,000 lines) because it is mostly API-agnostic logic:

- **Pass declaration** — Names, inputs, outputs, and resource descriptions for each pass.
- **DAG construction and topological sort** — Building the directed acyclic graph and determining execution order.
- **Barrier / transition generation** — This is the API-specific part. D3D12 uses resource state transitions; Vulkan uses pipeline barriers with image layout transitions.
- **Transient resource allocation** — Aliasing GPU memory for resources whose lifetimes do not overlap, reducing memory usage.

### Real-World Calibration Points

- **Granite** (Themaister's Vulkan engine) — ~30,000+ lines. Well beyond "simplest possible" but an excellent reference for production-quality rendergraph design.
- **niagara** (Arseny Kapoulkine's Vulkan tutorial series) — ~3,000 lines. No render graph, but demonstrates the rendering and synchronization patterns you will need.
- A **bare-minimum rendergraph demo** with 2–3 hardcoded passes and no generality could be achieved in roughly 4,000–5,000 lines total.

**Realistic target**: If you keep scope tight, expect approximately **10,000 lines** for either API to get something that renders a loaded scene through a configurable pass graph.

---

## 5. Learning Resources for Rendergraph-Based Engines

### Foundational Theory — Start Here

These resources explain what a rendergraph is, why it exists, and how to think about designing one. Read or watch these before writing any rendergraph code.

1. **"FrameGraph: Extensible Rendering Architecture in Frostbite"** — Yuriy O'Donnell (GDC 2017)
   - The seminal industry talk on render graphs. Explains the motivation (taming complexity in large rendering pipelines), the design (declare-then-compile-then-execute), and the implementation in Frostbite.
   - Search for the recording and slides on the **GDC Vault**.

2. **"Organizing GPU Work with Directed Acyclic Graphs"** — Pavlo Muratov (2020, blog post)
   - An excellent beginner-friendly writeup that walks through building a rendergraph step by step, from scratch. Covers resource lifetime tracking, automatic barrier placement, and transient resource allocation.
   - This is the best bridge between theory and implementation for someone who has never built a rendergraph.

3. **"Render Graphs and Vulkan — A Deep Dive"** — Themaister / Hans-Kristian Arntzen (blog)
   - Bridges rendergraph theory to Vulkan-specific implementation details. Discusses how Vulkan's synchronization primitives map to rendergraph concepts.

### Vulkan-Specific Resources

| Resource | Description |
|---|---|
| **[vulkan-tutorial.com](https://vulkan-tutorial.com)** | The standard beginner tutorial for Vulkan. Covers the full API basics (instance, device, swapchain, pipelines, rendering) from scratch. Complete this before attempting a rendergraph — you need to understand the API primitives first. |
| **[vkguide.dev](https://vkguide.dev)** (Chapter 2 / new edition) | A modern Vulkan guide that uses newer extensions and skips legacy patterns entirely. The updated version uses `VK_KHR_dynamic_rendering` (no render pass objects), `VK_KHR_synchronization2` (cleaner barriers), and descriptor buffers. Highly recommended for learning modern Vulkan idioms. |
| **Arseny Kapoulkine's "niagara" series** (YouTube + GitHub) | A video series that builds a Vulkan renderer incrementally, explaining each design decision. Does not implement a rendergraph per se, but teaches the GPU resource management and synchronization patterns that underpin one. |
| **Granite engine** (GitHub, by Themaister) | A real-world open-source Vulkan engine with a well-structured `RenderGraph` module. Excellent reference code for studying how a production rendergraph is organized. The codebase is large, so focus on the render graph module specifically. |

#### Key Modern Vulkan Features to Learn Alongside

These extensions simplify Vulkan significantly and are a natural fit for rendergraph architectures:

- **`VK_KHR_dynamic_rendering`** — Eliminates the need for `VkRenderPass` and `VkFramebuffer` objects. You specify attachments inline when beginning rendering. This removes a major source of boilerplate and aligns perfectly with how a rendergraph declares passes.
- **`VK_KHR_synchronization2`** — A cleaner, more expressive barrier and synchronization API. Pipeline stage and access flags are combined into a single structure, making it much easier to generate correct barriers from rendergraph transitions.
- **`VK_EXT_descriptor_buffer`** — Enables bindless descriptor access by placing descriptors directly in buffer memory. Simplifies descriptor management compared to traditional descriptor sets and pools.

### D3D12-Specific Resources

| Resource | Description |
|---|---|
| **Microsoft's DirectX-Graphics-Samples** (GitHub) | The official D3D12 sample repository. Includes **MiniEngine**, which contains a frame-graph-like rendering structure. Good for understanding how D3D12 idioms map to higher-level rendering architectures. |
| **[3dgep.com](https://3dgep.com) — "Learning DirectX 12"** (Jeremiah van Oosten) | A thorough multi-part tutorial series that covers modern D3D12 from scratch. Walks through device creation, command lists, resource management, and rendering step by step. |
| **"D3D12 Work Graphs"** — AMD GPUOpen blog posts | Covers the newer D3D12 Work Graphs feature (introduced with Shader Model 6.8). Work Graphs are distinct from render graphs — they enable GPU-driven dispatch and scheduling — but they are relevant to understanding the direction of modern GPU-driven rendering. |
| **Wicked Engine** (GitHub, by turanszkij) | An open-source game engine with a clean rendergraph implementation. The author has accompanying blog posts explaining the design and evolution of the engine's rendering architecture. |

#### Key Modern D3D12 Features to Learn Alongside

- **Enhanced Barriers** — Replaces the legacy `D3D12_RESOURCE_BARRIER` model with a more expressive system that better matches how rendergraphs reason about resource state transitions. Supports more granular synchronization and eliminates some of the pitfalls of the old barrier API.
- **Work Graphs (Shader Model 6.8+)** — A bleeding-edge feature for GPU-driven dispatch. The GPU itself decides what work to launch next, enabling fully GPU-driven rendering pipelines. Advanced, but worth understanding for future-oriented engine design.
- **Bindless Resources via Shader Model 6.6 Dynamic Resources** — Allows shaders to access resources without explicit binding. Simplifies descriptor management and pairs well with rendergraph architectures that manage resources at a higher level.

### Recommended Learning Order

```
1.  Learn the graphics API basics (pick one: Vulkan or D3D12)
    Use vulkan-tutorial.com or 3dgep.com respectively.
          ↓
2.  Watch the Frostbite FrameGraph GDC talk
    Understand the motivation and high-level design.
          ↓
3.  Read Pavlo Muratov's blog post
    Bridge from theory to implementation. Understand the data structures and algorithms.
          ↓
4.  Study one reference implementation
    Granite's RenderGraph module (Vulkan) or Wicked Engine (D3D12).
    Focus on how passes are declared, how the DAG is built, and how barriers are generated.
          ↓
5.  Build your own minimal rendergraph with 2–3 hardcoded passes
    Start concrete: e.g., shadow map pass → forward shading pass.
    Manually place barriers. Get it rendering correctly.
          ↓
6.  Generalize incrementally
    Extract the pattern into a reusable system. Add automatic barrier insertion,
    transient resource allocation, and dynamic pass registration one piece at a time.
```

### Practical Tip

Do not attempt to build a fully general rendergraph from day one. Start with a **hardcoded 2-pass graph** (e.g., shadow map → forward shading), manually insert your barriers, and get it rendering correctly. Then extract the common patterns into a reusable system. You will understand the "why" behind each abstraction far better than if you start by designing the most general system you can imagine.

---

*Last updated: March 2026*
