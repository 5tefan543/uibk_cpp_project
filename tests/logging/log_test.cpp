#ifndef _WIN32 // We're only testing log_test on linux as unsetenv() and setenv() are linux sys-calls,
               // the current test runner is a linux system and we want to avoid
               // e.g. another dependency for OS agnostic env. var. management.
               // std::getenv() should work fine on linux & windows.

#include "logging/log.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <format>
#include <iostream>
#include <sstream>
#include <string>

namespace {

struct OutRedirect {
    OutRedirect() : oldBufCout_(std::cout.rdbuf(ssCout_.rdbuf())), oldBufCerr_(std::cerr.rdbuf(ssCerr_.rdbuf())) {}

    ~OutRedirect()
    {
        std::cout.rdbuf(oldBufCout_);
        std::cerr.rdbuf(oldBufCerr_);
    }

    std::string getCout() const { return ssCout_.str(); }
    std::string getCerr() const { return ssCerr_.str(); }

  private:
    std::stringstream ssCout_;
    std::stringstream ssCerr_;
    std::streambuf *oldBufCout_;
    std::streambuf *oldBufCerr_;
};

struct EnvGuard {
    EnvGuard()
    {
        if (const char *level = std::getenv(logger::logLevelEnvVar)) {
            oldLevel_ = level;
        }
        if (const char *color = std::getenv(logger::logColorEnvVar)) {
            oldColor_ = color;
        }

        unsetenv(logger::logLevelEnvVar);
        unsetenv(logger::logColorEnvVar);
    }

    ~EnvGuard()
    {
        unsetenv(logger::logLevelEnvVar);
        unsetenv(logger::logColorEnvVar);

        if (!oldLevel_.empty()) {
            setenv(logger::logLevelEnvVar, oldLevel_.c_str(), 1);
        }
        if (!oldColor_.empty()) {
            setenv(logger::logColorEnvVar, oldColor_.c_str(), 1);
        }

        logger::configure();
    }

  private:
    std::string oldLevel_;
    std::string oldColor_;
};

std::string formatLogLine(logger::LogLevel level, bool useColor, const std::string &msg)
{
    const char *clear = useColor ? "\033[0m" : "";

    switch (level) {
    case logger::ERROR:
        return std::format("{}[ERROR] {}{}\n", useColor ? "\033[1;31m" : "", clear, msg);
    case logger::WARNING:
        return std::format("{}[WARN]  {}{}\n", useColor ? "\033[1;33m" : "", clear, msg);
    case logger::INFO:
        return std::format("{}[INFO]  {}{}\n", useColor ? "\033[1;32m" : "", clear, msg);
    case logger::DEBUG:
        return std::format("{}[DEBUG] {}{}\n", useColor ? "\033[1m" : "", clear, msg);
    case logger::SILENT:
    default:
        return "";
    }
}

} // namespace

TEST_CASE("Logger settings can be overridden by environment variables")
{
    EnvGuard envGuard; // is constructed / destroyed by each section

    const auto levels = std::array{logger::SILENT, logger::ERROR, logger::WARNING, logger::INFO, logger::DEBUG};
    const auto levelNames = std::array{"Silent", "Error", "Warning", "Info", "Debug"};

    SECTION("Log level env var overrides constructor level")
    {
        for (std::size_t i = 0; i < levels.size(); ++i) {
            REQUIRE(setenv(logger::logLevelEnvVar, levelNames[i], 1) == 0);

            const logger::LogSettings settings{logger::ERROR, true};

            REQUIRE(settings.getLevel() == levels[i]);
        }
    }

    SECTION("Log color env var overrides constructor color")
    {
        REQUIRE(setenv(logger::logColorEnvVar, "On", 1) == 0);
        REQUIRE(logger::LogSettings{logger::ERROR, false}.getUseColor());

        REQUIRE(setenv(logger::logColorEnvVar, "Off", 1) == 0);
        REQUIRE_FALSE(logger::LogSettings{logger::ERROR, true}.getUseColor());
    }

    SECTION("Invalid log level env var prints plain error to cerr")
    {
        REQUIRE(setenv(logger::logLevelEnvVar, "Invalid", 1) == 0);

        std::string coutOutput;
        std::string cerrOutput;

        {
            OutRedirect redirect;
            // create settings to trigger log level env var parsing
            const logger::LogSettings settings;
            coutOutput = redirect.getCout();
            cerrOutput = redirect.getCerr();
        }

        REQUIRE(coutOutput.empty());
        REQUIRE(cerrOutput
                == "[ERROR] Invalid environment logging level: 'Invalid'\n"
                   "Valid values are: Silent, Error, Warning, Info, Debug\n");
    }

    SECTION("Invalid log color env var prints plain error to cerr")
    {
        REQUIRE(setenv(logger::logColorEnvVar, "Invalid", 1) == 0);

        std::string coutOutput;
        std::string cerrOutput;

        {
            OutRedirect redirect;
            // create settings to trigger log color env var parsing
            const logger::LogSettings settings;
            coutOutput = redirect.getCout();
            cerrOutput = redirect.getCerr();
        }

        REQUIRE(coutOutput.empty());
        REQUIRE(cerrOutput
                == "[ERROR] Invalid environment logging color setting: 'Invalid'\n"
                   "Valid values are: On, Off\n");
    }

    SECTION("Logger prints expected messages depending on configured level")
    {
        for (bool useColor : {false, true}) {
            for (std::size_t i = 0; i < levels.size(); ++i) {
                logger::configure(levels[i], useColor);

                for (std::size_t messageIndex = 0; messageIndex < levels.size(); ++messageIndex) {
                    const auto messageLevel = levels[messageIndex];
                    const auto msg = std::format("{}-log-test", levelNames[messageIndex]);

                    std::string coutOutput;
                    std::string cerrOutput;

                    {
                        OutRedirect redirect;
                        logger::log(messageLevel, msg);
                        coutOutput = redirect.getCout();
                        cerrOutput = redirect.getCerr();
                    }

                    INFO(std::format("configured={}, message={}, useColor={}", levelNames[i], levelNames[messageIndex],
                                     useColor));

                    if (messageLevel == logger::SILENT || messageLevel > levels[i]) {
                        REQUIRE(coutOutput.empty());
                        REQUIRE(cerrOutput.empty());
                    } else if (messageLevel == logger::ERROR) {
                        REQUIRE(coutOutput.empty());
                        REQUIRE(cerrOutput == formatLogLine(messageLevel, useColor, msg));
                    } else {
                        REQUIRE(coutOutput == formatLogLine(messageLevel, useColor, msg));
                        REQUIRE(cerrOutput.empty());
                    }
                }
            }
        }
    }
}

#endif