export module dod:lesson04;

import std;

// Lesson 4: A toy frame graph.
//
// Passes are data. We declare what each pass reads and writes, and a
// simple scheduler topologically sorts them and prints the "barriers"
// it would insert between passes that share a resource.
//
// The point isn't the barrier algorithm (this one is deliberately naive);
// it's that rendering *order and synchronisation* become the output of a
// compiler, not hand-written imperative code.

export namespace dod::lesson04
{
    struct Pass
    {
        std::string name;
        std::vector<std::string> reads;
        std::vector<std::string> writes;
        std::function<void()> execute;
    };

    struct ScheduledStep
    {
        enum class Kind { Barrier, Execute };
        Kind kind;
        std::string detail;  // resource name for Barrier, pass name for Execute
    };

    // Very naive scheduler:
    //  * Passes are ordered as declared (assumes user wrote them in a sensible order).
    //  * Between consecutive passes, for every resource that was written and is now
    //    read, emit a Barrier step naming the resource.
    // A real frame graph also does: alias analysis, parallel pass detection,
    // transient resource lifetime tracking, queue scheduling, async compute overlap.
    inline std::vector<ScheduledStep> Compile(std::span<const Pass> passes)
    {
        std::vector<ScheduledStep> steps;
        std::unordered_set<std::string> writtenSoFar;

        for (const auto& p : passes)
        {
            for (const auto& r : p.reads)
                if (writtenSoFar.contains(r))
                    steps.push_back({ ScheduledStep::Kind::Barrier, r });

            steps.push_back({ ScheduledStep::Kind::Execute, p.name });

            for (const auto& w : p.writes)
                writtenSoFar.insert(w);
        }
        return steps;
    }

    inline void Execute(std::span<const Pass> passes, std::span<const ScheduledStep> steps)
    {
        std::unordered_map<std::string, const Pass*> byName;
        for (const auto& p : passes)
            byName[p.name] = &p;

        for (const auto& s : steps)
        {
            switch (s.kind)
            {
            case ScheduledStep::Kind::Barrier:
                std::println("    [barrier] resource='{}'", s.detail);
                break;
            case ScheduledStep::Kind::Execute:
                std::println("    [execute] pass='{}'", s.detail);
                if (auto it = byName.find(s.detail); it != byName.end() && it->second->execute)
                    it->second->execute();
                break;
            }
        }
    }

    inline void Run()
    {
        std::println("\n=== Lesson 04: Toy frame graph ===");

        std::vector<Pass> passes;
        passes.push_back(Pass{
            "shadow-map",
            /*reads*/  {},
            /*writes*/ { "ShadowMap" },
            [] { /* pretend to rasterise shadow casters */ },
        });
        passes.push_back(Pass{
            "depth-prepass",
            /*reads*/  {},
            /*writes*/ { "DepthBuffer" },
            [] {},
        });
        passes.push_back(Pass{
            "gbuffer",
            /*reads*/  { "DepthBuffer" },  // uses depth from prepass for early-Z
            /*writes*/ { "GBufferAlbedo", "GBufferNormal" },
            [] {},
        });
        passes.push_back(Pass{
            "lighting",
            /*reads*/  { "GBufferAlbedo", "GBufferNormal", "DepthBuffer", "ShadowMap" },
            /*writes*/ { "LitColor" },
            [] {},
        });
        passes.push_back(Pass{
            "tonemap",
            /*reads*/  { "LitColor" },
            /*writes*/ { "Swapchain" },
            [] {},
        });

        std::println("  Declared passes:");
        for (const auto& p : passes)
            std::println("    - {:<14} reads=[{}] writes=[{}]",
                p.name,
                [&] { std::string s; for (auto& r : p.reads)  { if (!s.empty()) s += ','; s += r; } return s; }(),
                [&] { std::string s; for (auto& w : p.writes) { if (!s.empty()) s += ','; s += w; } return s; }());

        const auto steps = Compile(passes);
        std::println("\n  Compiled schedule:");
        Execute(passes, steps);
        std::println("\n  Lesson: adding a pass is appending to a list.");
        std::println("          The scheduler derives barriers from read/write sets — you never");
        std::println("          write vkCmdPipelineBarrier2 by hand again.");
    }
}
