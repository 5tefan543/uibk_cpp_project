#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>

namespace test {

namespace fs = std::filesystem;

class ScopedTestDirectory {
  public:
    explicit ScopedTestDirectory(std::string_view prefix)
        : oldPath_(fs::current_path()), testPath_(createUniqueTempDirectory(prefix))
    {
        fs::current_path(testPath_);
    }

    ~ScopedTestDirectory()
    {
        std::error_code ec;
        fs::current_path(oldPath_, ec);
        fs::remove_all(testPath_, ec);
    }

    const fs::path &path() const { return testPath_; }

  private:
    static fs::path createUniqueTempDirectory(std::string_view prefix)
    {
        const auto uniquePart = std::chrono::steady_clock::now().time_since_epoch().count();
        const fs::path dir = fs::temp_directory_path() / (std::string(prefix) + std::to_string(uniquePart));
        fs::create_directories(dir);
        return dir;
    }

    fs::path oldPath_;
    fs::path testPath_;
};

} // namespace test
