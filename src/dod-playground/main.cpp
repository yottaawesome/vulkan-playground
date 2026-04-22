import std;
import dod;

namespace
{
    void PrintHelp()
    {
        std::println("dod-playground — study data-oriented design without the GPU ceremony.");
        std::println("");
        std::println("Usage:");
        std::println("  dod-playground.exe            Run all lessons.");
        std::println("  dod-playground.exe <n>        Run lesson N (1..4).");
        std::println("  dod-playground.exe -h|--help  Show this help.");
        std::println("");
        std::println("Lessons:");
        std::println("  1  AoS vs SoA          — data layout and the cache");
        std::println("  2  Sort keys           — order is data; pick it to minimise binds");
        std::println("  3  Indirect rendering  — cull pass -> compact batch -> single submit");
        std::println("  4  Frame graph         — passes are data; barriers are derived");
        std::println("");
        std::println("Tip: build Release for realistic numbers (Debug bounds-checks dominate Lesson 1).");
    }
}

int main(int argc, char** argv)
{
    if (argc >= 2)
    {
        const std::string_view arg{ argv[1] };
        if (arg == "-h" || arg == "--help")
        {
            PrintHelp();
            return 0;
        }

        int which = 0;
        if (auto [p, ec] = std::from_chars(arg.data(), arg.data() + arg.size(), which);
            ec != std::errc{})
        {
            std::println("error: expected a lesson number (1..4) or --help, got '{}'", arg);
            return 1;
        }

        switch (which)
        {
        case 1: dod::lesson01::Run(); return 0;
        case 2: dod::lesson02::Run(); return 0;
        case 3: dod::lesson03::Run(); return 0;
        case 4: dod::lesson04::Run(); return 0;
        default:
            std::println("error: no such lesson '{}'. Valid range: 1..4.", which);
            return 1;
        }
    }

    std::println("dod-playground — running all lessons.");
    std::println("(Pass --help for per-lesson run, or a number 1..4.)");
    dod::lesson01::Run();
    dod::lesson02::Run();
    dod::lesson03::Run();
    dod::lesson04::Run();
    std::println("\nDone.");
    return 0;
}
