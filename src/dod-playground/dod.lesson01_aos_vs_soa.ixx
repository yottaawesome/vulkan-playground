export module dod:lesson01;

import std;
import :math;
import :timer;

// Lesson 1: AoS vs SoA for the hot path (frustum culling).
//
// Same work, same data, two layouts. The only change is how the fields are
// arranged in memory. Expect SoA to win by a large factor in release builds.
//
// Run this with /O2 (Release) to see realistic numbers. Debug builds still
// show the trend, but the absolute times are dominated by bounds checks.

export namespace dod::lesson01
{
    // ---------- AoS: what you'd naturally write in OO code ----------
    struct EntityAoS
    {
        // Hot: accessed every frame by culling.
        Vec3 boundsCenter;
        float boundsRadius;

        // Cold: almost never read in the frame loop, but sits right next to
        // the hot data on every cache line, polluting the cache.
        std::array<float, 16> transform{};   // 64 B — pretend model matrix
        std::array<char, 32> debugName{};
        std::uint64_t lastEditedNs{};
        void* scriptHandle{};
        std::array<std::uint32_t, 8> padding{};  // "other cold metadata"
    };

    // ---------- SoA: hot fields packed, cold fields elsewhere ----------
    struct EntitiesSoA
    {
        std::vector<Vec3> boundsCenter;
        std::vector<float> boundsRadius;

        struct Cold
        {
            std::vector<std::array<float, 16>> transform;
            std::vector<std::array<char, 32>> debugName;
            std::vector<std::uint64_t> lastEditedNs;
            std::vector<void*> scriptHandle;
        } cold;

        std::size_t Size() const noexcept { return boundsCenter.size(); }
    };

    inline std::vector<EntityAoS> MakeEntitiesAoS(std::size_t n, std::uint32_t seed)
    {
        std::mt19937 rng{ seed };
        std::uniform_real_distribution<float> pos{ -100.0f, 100.0f };
        std::uniform_real_distribution<float> rad{ 0.1f, 2.0f };

        std::vector<EntityAoS> out(n);
        for (auto& e : out)
        {
            e.boundsCenter = { pos(rng), pos(rng), pos(rng) };
            e.boundsRadius = rad(rng);
        }
        return out;
    }

    inline EntitiesSoA MakeEntitiesSoA(std::size_t n, std::uint32_t seed)
    {
        std::mt19937 rng{ seed };
        std::uniform_real_distribution<float> pos{ -100.0f, 100.0f };
        std::uniform_real_distribution<float> rad{ 0.1f, 2.0f };

        EntitiesSoA out;
        out.boundsCenter.resize(n);
        out.boundsRadius.resize(n);
        out.cold.transform.resize(n);
        out.cold.debugName.resize(n);
        out.cold.lastEditedNs.resize(n);
        out.cold.scriptHandle.resize(n);
        for (std::size_t i = 0; i < n; ++i)
        {
            out.boundsCenter[i] = { pos(rng), pos(rng), pos(rng) };
            out.boundsRadius[i] = rad(rng);
        }
        return out;
    }

    inline std::uint64_t CullAoS(std::span<const EntityAoS> entities, const Frustum& f) noexcept
    {
        std::uint64_t visible = 0;
        for (const auto& e : entities)
            if (f.ContainsSphere(e.boundsCenter, e.boundsRadius))
                ++visible;
        return visible;
    }

    inline std::uint64_t CullSoA(const EntitiesSoA& e, const Frustum& f) noexcept
    {
        std::uint64_t visible = 0;
        const std::size_t n = e.Size();
        for (std::size_t i = 0; i < n; ++i)
            if (f.ContainsSphere(e.boundsCenter[i], e.boundsRadius[i]))
                ++visible;
        return visible;
    }

    inline void Run()
    {
        std::println("\n=== Lesson 01: AoS vs SoA (frustum culling) ===");
        std::println("  EntityAoS size: {} bytes (hot fields only fill ~16 B)", sizeof(EntityAoS));
        std::println("  Culling 200,000 entities, 50 iterations per layout.\n");

        constexpr std::size_t N = 200'000;
        constexpr std::size_t Iters = 50;
        const Frustum frustum = MakeBoxFrustum(40.0f);

        const auto aos = MakeEntitiesAoS(N, 0x1234);
        const auto soa = MakeEntitiesSoA(N, 0x1234);

        const auto r1 = Benchmark("AoS cull (hot+cold interleaved)", Iters, [&] {
            return CullAoS(aos, frustum);
        });
        const auto r2 = Benchmark("SoA cull (hot fields packed)", Iters, [&] {
            return CullSoA(soa, frustum);
        });

        PrintBench(r1);
        PrintBench(r2);

        const double speedup = r1.minUs / r2.minUs;
        std::println("\n  SoA is {:.2f}x faster (min vs min).", speedup);
        std::println("  Same algorithm, same results. Only the data layout changed.");
    }
}
