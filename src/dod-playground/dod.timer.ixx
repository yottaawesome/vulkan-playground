export module dod:timer;

import std;

export namespace dod
{
    // Runs a callable N times, reports min/median/mean of wall time in microseconds.
    // We take the min as the headline because scheduler noise only ever makes runs slower.
    struct BenchResult
    {
        std::string name;
        double minUs{};
        double medianUs{};
        double meanUs{};
        std::size_t iterations{};
        // A sink value, printed to defeat dead-code elimination of the measured work.
        std::uint64_t sink{};
    };

    template <typename F>
    BenchResult Benchmark(std::string name, std::size_t iterations, F&& fn)
    {
        std::vector<double> samples;
        samples.reserve(iterations);
        std::uint64_t sinkAccum = 0;

        for (std::size_t i = 0; i < iterations; ++i)
        {
            const auto start = std::chrono::steady_clock::now();
            const std::uint64_t s = fn();
            const auto end = std::chrono::steady_clock::now();
            sinkAccum ^= s;
            const auto us = std::chrono::duration<double, std::micro>(end - start).count();
            samples.push_back(us);
        }

        std::ranges::sort(samples);
        const double minUs = samples.front();
        const double medianUs = samples[samples.size() / 2];
        const double meanUs =
            std::accumulate(samples.begin(), samples.end(), 0.0) / static_cast<double>(samples.size());

        return BenchResult{ std::move(name), minUs, medianUs, meanUs, iterations, sinkAccum };
    }

    inline void PrintBench(const BenchResult& r)
    {
        std::println("  {:<32} min={:>9.2f} us   median={:>9.2f} us   mean={:>9.2f} us   (iters={}, sink={})",
            r.name, r.minUs, r.medianUs, r.meanUs, r.iterations, r.sink);
    }
}
