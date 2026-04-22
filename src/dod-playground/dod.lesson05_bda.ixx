export module dod:lesson05;

import std;
import :math;
import :fakegpu;

// Lesson 5: Buffer Device Address (BDA), modelled with plain C++ pointers.
//
// In real Vulkan 1.2+ BDA, each GPU buffer has a 64-bit device address.
// Shaders receive that address (typically via a push constant) and
// dereference it like a C pointer — no descriptor set, no binding slot.
//
// On the CPU, a raw pointer *is* exactly that: a 64-bit address into an
// array. So we can model the whole ergonomic win without any GPU at all.
//
// Compare the shape of the "shader" functions here with what you'd write in
// GLSL with GL_EXT_buffer_reference. They are one-to-one.

export namespace dod::lesson05
{
    // ---------- "Resource tables" ----------
    // In a real renderer these would be VkBuffers with VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT.
    // Here they are just std::vectors — and a pointer into .data() is our device address.

    struct Transform
    {
        std::array<float, 16> matrix;  // pretend model matrix
    };

    struct Material
    {
        std::uint32_t albedoTextureIndex;  // would index a bindless texture array
        std::uint32_t flags;
        float         roughness;
        float         metallic;
    };

    struct MeshHeader
    {
        std::uint32_t firstIndex;
        std::uint32_t indexCount;
        std::uint32_t firstVertex;
        std::uint32_t _pad;
    };

    // ---------- The scene pointer graph ----------
    //
    // A single "SceneData" struct holds pointers to every table the shader
    // might need. The CPU writes *one* push-constant-sized struct per frame.
    // The shader (or in our case, the "draw" function) walks from there.
    //
    // In GLSL this would be:
    //   layout(buffer_reference) buffer TransformBuf { Transform t[]; };
    //   layout(buffer_reference) buffer MaterialBuf  { Material  m[]; };
    //   layout(push_constant) uniform PC { SceneData scene; };

    struct SceneData
    {
        const Transform*    transforms;   // -> big array, one per instance
        const Material*     materials;    // -> one per material id
        const MeshHeader*   meshes;       // -> one per mesh id
        std::uint32_t       transformCount;
        std::uint32_t       materialCount;
        std::uint32_t       meshCount;
    };

    // Per-draw record that the CPU (or a GPU cull pass) writes into an indirect
    // buffer. Note there are *no descriptor set indices*, just ids that the
    // shader uses to index tables it got pointers to.
    struct IndirectCmd
    {
        std::uint32_t meshId;
        std::uint32_t materialId;
        std::uint32_t transformId;
        std::uint32_t pipelineId;
    };

    // ---------- Scene construction ----------

    struct SceneStorage
    {
        std::vector<Transform>  transforms;
        std::vector<Material>   materials;
        std::vector<MeshHeader> meshes;
    };

    inline SceneStorage MakeScene(std::size_t instances, std::uint32_t seed)
    {
        std::mt19937 rng{ seed };
        std::uniform_real_distribution<float> f01{ 0.0f, 1.0f };
        std::uniform_int_distribution<std::uint32_t> texIdx{ 0, 255 };

        SceneStorage s;
        s.transforms.resize(instances);
        s.materials.resize(32);
        s.meshes.resize(64);

        for (auto& t : s.transforms)
            for (auto& v : t.matrix)
                v = f01(rng);

        for (auto& m : s.materials)
            m = { texIdx(rng), 0u, f01(rng), f01(rng) };

        std::uint32_t cursor = 0;
        for (auto& mh : s.meshes)
        {
            const std::uint32_t count = 32 * 3;
            mh = { cursor, count, cursor, 0 };
            cursor += count;
        }
        return s;
    }

    inline SceneData MakeSceneView(const SceneStorage& s)
    {
        // This is where the rubber meets the road: on real Vulkan, each of
        // these pointers would be a VkDeviceAddress returned by
        // vkGetBufferDeviceAddress(). Here we use .data() — same concept,
        // different address space.
        return SceneData{
            s.transforms.data(),
            s.materials.data(),
            s.meshes.data(),
            static_cast<std::uint32_t>(s.transforms.size()),
            static_cast<std::uint32_t>(s.materials.size()),
            static_cast<std::uint32_t>(s.meshes.size()),
        };
    }

    // ---------- The "shader" ----------
    //
    // Compare this to a GLSL vertex shader that takes `SceneData scene` as a
    // push constant and `IndirectCmd cmd` as per-draw data. The body is
    // identical: dereference pointers, read fields, no binding calls.

    struct ShadedVertex  // stand-in for gl_Position + varyings
    {
        float worldZ;
        std::uint32_t albedoTextureIndex;
    };

    inline ShadedVertex FakeVertexShader(const SceneData& scene, const IndirectCmd& cmd) noexcept
    {
        // These three lines are the whole point of BDA.
        const Transform&  xform    = scene.transforms[cmd.transformId];
        const Material&   material = scene.materials[cmd.materialId];
        const MeshHeader& mesh     = scene.meshes[cmd.meshId];

        // Pretend to transform a vertex. The 14th matrix element is our "Z".
        return ShadedVertex{
            xform.matrix[14] + static_cast<float>(mesh.firstVertex) * 0.0f,
            material.albedoTextureIndex,
        };
    }

    // ---------- Compare-and-contrast: descriptor-set-style draw ----------
    //
    // To highlight what BDA removes, here is the same draw expressed the
    // old way. Every draw has to resolve which descriptor set is currently
    // bound (simulated by a lookup), costing a (trivial) "state change".

    struct DescriptorStyleBindings
    {
        const Transform* boundTransformSet;
        const Material*  boundMaterialSet;
        const MeshHeader* boundMeshSet;
    };

    // ---------- Driver: run both styles and compare binds ----------

    inline std::vector<IndirectCmd> MakeDraws(
        std::size_t count,
        const SceneStorage& scene,
        std::uint32_t seed)
    {
        std::mt19937 rng{ seed };
        std::uniform_int_distribution<std::uint32_t> mesh{ 0, static_cast<std::uint32_t>(scene.meshes.size()) - 1 };
        std::uniform_int_distribution<std::uint32_t> mat{ 0, static_cast<std::uint32_t>(scene.materials.size()) - 1 };
        std::uniform_int_distribution<std::uint32_t> xform{ 0, static_cast<std::uint32_t>(scene.transforms.size()) - 1 };
        std::uniform_int_distribution<std::uint32_t> pipe{ 0, 3 };

        std::vector<IndirectCmd> out(count);
        for (auto& c : out)
            c = { mesh(rng), mat(rng), xform(rng), pipe(rng) };
        return out;
    }

    inline void Run()
    {
        std::println("\n=== Lesson 05: Buffer Device Address (BDA) ===");
        std::println("  Modelling BDA with raw C++ pointers. In real Vulkan 1.2+ each");
        std::println("  'pointer' here would be a VkDeviceAddress from vkGetBufferDeviceAddress.\n");

        constexpr std::size_t Instances = 10'000;
        constexpr std::size_t Draws     = 10'000;

        const auto storage = MakeScene(Instances, 0xB0A);
        const auto scene   = MakeSceneView(storage);
        auto draws         = MakeDraws(Draws, storage, 0xD4);

        // Sort once so both styles see the same opportunity for batching.
        std::ranges::sort(draws, [](const IndirectCmd& a, const IndirectCmd& b) {
            return std::tie(a.pipelineId, a.materialId, a.meshId)
                 < std::tie(b.pipelineId, b.materialId, b.meshId);
        });

        // --- BDA style: one SceneData pointer struct, per-draw is pure data ---
        {
            FakeGpu gpu;
            std::uint64_t sink = 0;
            // "Bind" happens once: the push constant carrying scene pointers.
            std::uint64_t bdaBinds = 1;
            for (const auto& c : draws)
            {
                const auto sv = FakeVertexShader(scene, c);
                sink ^= std::bit_cast<std::uint32_t>(sv.worldZ) ^ sv.albedoTextureIndex;
                // Pipeline is the only thing that can still cause a real bind.
                gpu.Draw(FakeDrawCall{ c.pipelineId, /*mat*/ 0, /*mesh*/ 0, 0.0f });
            }
            std::println("  BDA style:");
            std::println("    scene-pointer pushes : {}", bdaBinds);
            std::println("    pipeline changes     : {}", gpu.stateChanges);
            std::println("    draws                : {}  (sink={})", gpu.drawCount, sink);
        }

        // --- Descriptor-set style: every material/mesh change is a bind ---
        {
            FakeGpu gpu;
            std::uint32_t lastMat = ~0u;
            std::uint32_t lastMesh = ~0u;
            std::uint64_t descriptorBinds = 0;
            for (const auto& c : draws)
            {
                if (c.materialId != lastMat) { ++descriptorBinds; lastMat = c.materialId; }
                if (c.meshId     != lastMesh){ ++descriptorBinds; lastMesh = c.meshId; }
                gpu.Draw(FakeDrawCall{ c.pipelineId, c.materialId, c.meshId, 0.0f });
            }
            std::println("  Descriptor-set style:");
            std::println("    descriptor binds     : {}", descriptorBinds);
            std::println("    total state changes  : {}", gpu.stateChanges);
            std::println("    draws                : {}", gpu.drawCount);
        }

        std::println("\n  Lesson: once the shader has *pointers*, per-draw CPU work collapses");
        std::println("          to 'write a small indirect record'. The descriptor plumbing");
        std::println("          for buffers disappears entirely. Textures still need bindless.");
    }
}
