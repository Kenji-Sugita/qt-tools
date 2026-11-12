#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

enum class OrderMode {
    Younger,
    Older
};

constexpr int ExitOrdered = 0;
constexpr int ExitUnordered = 1;
constexpr int ExitError = 2;

std::string programName(const char* argv0)
{
    return fs::path(argv0).filename().string();
}

void printUsage(const std::string& name)
{
    std::cerr << "usage: " << name << " FILE FILE...\n";
}

bool resolveMode(const std::string& name, OrderMode& mode)
{
    if (name == "younger") {
        mode = OrderMode::Younger;
        return true;
    }
    if (name == "older") {
        mode = OrderMode::Older;
        return true;
    }
    return false;
}

bool isOrdered(const fs::file_time_type& left, const fs::file_time_type& right, OrderMode mode)
{
    if (mode == OrderMode::Younger)
        return left >= right;
    return left <= right;
}

bool getLastWriteTime(const std::string& path, fs::file_time_type& time)
{
    std::error_code error;
    time = fs::last_write_time(path, error);
    return !error;
}

} // namespace

int main(int argc, char* argv[])
{
    const std::string name = argc > 0 ? programName(argv[0]) : std::string();

    OrderMode mode = OrderMode::Younger;
    if (!resolveMode(name, mode)) {
        printUsage(name);
        return ExitError;
    }

    if (argc < 3) {
        printUsage(name);
        return ExitError;
    }

    fs::file_time_type previousTime;
    if (!getLastWriteTime(argv[1], previousTime)) {
        std::cerr << name << ": cannot stat '" << argv[1] << "'\n";
        return ExitError;
    }

    for (int i = 2; i < argc; ++i) {
        fs::file_time_type currentTime;
        if (!getLastWriteTime(argv[i], currentTime)) {
            std::cerr << name << ": cannot stat '" << argv[i] << "'\n";
            return ExitError;
        }

        if (!isOrdered(previousTime, currentTime, mode)) {
            std::cerr << name << ": order violation between '"
                      << argv[i - 1] << "' and '" << argv[i] << "'\n";
            return ExitUnordered;
        }

        previousTime = currentTime;
    }

    return ExitOrdered;
}
