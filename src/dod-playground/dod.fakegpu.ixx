export module dod:fakegpu;

import std;

export namespace dod
{
    // A "draw call" stripped of all API detail. The only things that affect our
    // proxy cost are pipeline/material/mesh bindings and draw ordering.
    struct FakeDrawCall
    {
        std::uint32_t pipelineId{};
        std::uint32_t materialId{};
        std::uint32_t meshId{};
        float depth{};  // view-space z, used for sorting opaque front-to-back or transparent back-to-front.
    };

    // Pretend GPU. Every state change (pipeline/material/mesh swap) costs 1.
    // That counter is our proxy for real-world bind overhead.
    struct FakeGpu
    {
        std::uint64_t stateChanges = 0;
        std::uint64_t drawCount = 0;
        std::uint32_t lastPipeline = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t lastMaterial = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t lastMesh = std::numeric_limits<std::uint32_t>::max();

        void Draw(const FakeDrawCall& d) noexcept
        {
            if (d.pipelineId != lastPipeline) { ++stateChanges; lastPipeline = d.pipelineId; }
            if (d.materialId != lastMaterial) { ++stateChanges; lastMaterial = d.materialId; }
            if (d.meshId     != lastMesh)     { ++stateChanges; lastMesh     = d.meshId; }
            ++drawCount;
        }

        void Reset() noexcept
        {
            stateChanges = 0;
            drawCount = 0;
            lastPipeline = std::numeric_limits<std::uint32_t>::max();
            lastMaterial = std::numeric_limits<std::uint32_t>::max();
            lastMesh = std::numeric_limits<std::uint32_t>::max();
        }
    };

    // Pack a 64-bit sort key: [pipeline:16 | material:16 | depth:32].
    // Sorting by this key groups pipelines, then materials, then draws front-to-back
    // within each bucket — the canonical opaque render order.
    inline std::uint64_t MakeOpaqueSortKey(const FakeDrawCall& d) noexcept
    {
        const std::uint64_t pipeline = static_cast<std::uint16_t>(d.pipelineId);
        const std::uint64_t material = static_cast<std::uint16_t>(d.materialId);
        // Depth encoded as uint so that ascending sort = front-to-back for positive z.
        const std::uint32_t depthBits = std::bit_cast<std::uint32_t>(d.depth);
        return (pipeline << 48) | (material << 32) | depthBits;
    }
}
