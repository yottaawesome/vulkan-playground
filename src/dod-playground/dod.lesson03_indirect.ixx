export module dod:lesson03;

import std;
import :math;
import :fakegpu;

// Lesson 3: CPU-side culling that emits an "indirect command buffer".
//
// The shape of this code is the same as GPU-driven rendering with
// vkCmdDrawIndexedIndirect / ExecuteIndirect: a big SoA of candidates,
// a parallel-safe filtering pass that writes a compact output array,
// and a single "submit the whole batch" call at the end.

export namespace dod::lesson03
{
    struct Scene
    {
        std::vector<Vec3>          centers;
        std::vector<float>         radii;
        std::vector<std::uint32_t> meshIds;
        std::vector<std::uint32_t> materialIds;
        std::vector<std::uint32_t> pipelineIds;
    };

    struct IndirectCmd
    {
        std::uint32_t meshId;
        std::uint32_t materialId;
        std::uint32_t pipelineId;
        std::uint32_t instanceIndex;  // index back into the Scene SoA
    };

    inline Scene MakeScene(std::size_t n, std::uint32_t seed)
    {
        std::mt19937 rng{ seed };
        std::uniform_real_distribution<float> pos{ -100.0f, 100.0f };
        std::uniform_real_distribution<float> rad{ 0.2f, 1.5f };
        std::uniform_int_distribution<std::uint32_t> pipelines{ 0, 3 };
        std::uniform_int_distribution<std::uint32_t> materials{ 0, 31 };
        std::uniform_int_distribution<std::uint32_t> meshes{ 0, 127 };

        Scene s;
        s.centers.resize(n);
        s.radii.resize(n);
        s.meshIds.resize(n);
        s.materialIds.resize(n);
        s.pipelineIds.resize(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            s.centers[i] = { pos(rng), pos(rng), pos(rng) };
            s.radii[i] = rad(rng);
            s.meshIds[i] = meshes(rng);
            s.materialIds[i] = materials(rng);
            s.pipelineIds[i] = pipelines(rng);
        }
        return s;
    }

    // Cull pass: reads SoA, writes a compact indirect buffer.
    // This is exactly the job a GPU compute shader would do in a real engine.
    inline void BuildIndirectBuffer(
        const Scene& scene,
        const Frustum& frustum,
        std::vector<IndirectCmd>& outCmds)
    {
        outCmds.clear();
        outCmds.reserve(scene.centers.size());
        const std::size_t n = scene.centers.size();
        for (std::size_t i = 0; i < n; ++i)
        {
            if (frustum.ContainsSphere(scene.centers[i], scene.radii[i]))
            {
                outCmds.push_back(IndirectCmd{
                    scene.meshIds[i],
                    scene.materialIds[i],
                    scene.pipelineIds[i],
                    static_cast<std::uint32_t>(i),
                });
            }
        }
    }

    inline void Run()
    {
        std::println("\n=== Lesson 03: CPU-driven indirect rendering ===");
        constexpr std::size_t N = 50'000;
        const auto scene = MakeScene(N, 0xBEEF);
        const Frustum frustum = MakeBoxFrustum(40.0f);

        std::vector<IndirectCmd> cmds;
        BuildIndirectBuffer(scene, frustum, cmds);

        // Sort before submit: group by pipeline, then material, then mesh.
        std::ranges::sort(cmds, [](const IndirectCmd& a, const IndirectCmd& b) {
            return std::tie(a.pipelineId, a.materialId, a.meshId)
                 < std::tie(b.pipelineId, b.materialId, b.meshId);
        });

        FakeGpu gpu;
        for (const auto& c : cmds)
            gpu.Draw(FakeDrawCall{ c.pipelineId, c.materialId, c.meshId, 0.0f });

        std::println("  Scene size       : {}", N);
        std::println("  Passed cull      : {}  ({:.1f}%)",
            cmds.size(),
            100.0 * static_cast<double>(cmds.size()) / static_cast<double>(N));
        std::println("  Submit-batch size: {} indirect commands", cmds.size());
        std::println("  State changes    : {}", gpu.stateChanges);
        std::println("  Lesson: the 'draw' stage is now a single loop over one contiguous array.");
        std::println("          All per-entity branching lives in the earlier cull pass, where");
        std::println("          it can be vectorised or moved to a compute shader.");
    }
}
