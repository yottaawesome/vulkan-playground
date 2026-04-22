export module dod:lesson02;

import std;
import :fakegpu;
import :timer;

// Lesson 2: Sort-key-based draw ordering.
//
// Generate a pile of random draw calls, submit them unsorted vs sorted by
// a packed [pipeline|material|depth] key, and compare state-change counts.
// State changes are our proxy for GPU bind cost.

export namespace dod::lesson02
{
    inline std::vector<FakeDrawCall> MakeDraws(std::size_t n, std::uint32_t seed)
    {
        std::mt19937 rng{ seed };
        std::uniform_int_distribution<std::uint32_t> pipelines{ 0, 7 };     // 8 pipelines
        std::uniform_int_distribution<std::uint32_t> materials{ 0, 63 };    // 64 materials
        std::uniform_int_distribution<std::uint32_t> meshes{ 0, 511 };      // 512 meshes
        std::uniform_real_distribution<float> depth{ 0.1f, 100.0f };

        std::vector<FakeDrawCall> out(n);
        for (auto& d : out)
            d = { pipelines(rng), materials(rng), meshes(rng), depth(rng) };
        return out;
    }

    inline void Run()
    {
        std::println("\n=== Lesson 02: Sort-key ordering ===");
        constexpr std::size_t N = 10'000;
        auto draws = MakeDraws(N, 0xCAFEBABE);

        FakeGpu gpu;

        // --- Unsorted submission ---
        gpu.Reset();
        for (const auto& d : draws)
            gpu.Draw(d);
        const auto unsortedChanges = gpu.stateChanges;

        // --- Sorted submission ---
        auto sorted = draws;
        std::ranges::sort(sorted, {}, MakeOpaqueSortKey);
        gpu.Reset();
        for (const auto& d : sorted)
            gpu.Draw(d);
        const auto sortedChanges = gpu.stateChanges;

        std::println("  Draws submitted     : {}", N);
        std::println("  State changes (unsorted): {}", unsortedChanges);
        std::println("  State changes (sorted)  : {}", sortedChanges);
        std::println("  Reduction: {:.2f}x fewer binds.",
            static_cast<double>(unsortedChanges) / static_cast<double>(sortedChanges));
        std::println("  Lesson: draw *order* is data. Pick an order that minimises transitions.");
    }
}
