#ifndef _WIN32 // We're only testing on linux
#include "../src/logging/log.cpp"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

struct OutRedirect {
    OutRedirect() : oldBufCout_(std::cout.rdbuf(ssCout_.rdbuf())), oldBufCerr_(std::cerr.rdbuf(ssCerr_.rdbuf())) {}
    ~OutRedirect()
    {
        std::cout.rdbuf(oldBufCout_);
        std::cerr.rdbuf(oldBufCerr_);
    }
    std::string getCout() { return ssCout_.str(); }
    std::string getCerr() { return ssCerr_.str(); }

  private:
    std::stringstream ssCout_;
    std::stringstream ssCerr_;
    std::streambuf *oldBufCout_;
    std::streambuf *oldBufCerr_;
};

// Return {log-correct-output-msg, log-input-msg}
std::tuple<std::string, std::string> formatLogLine(logger::Level l, bool useColor, std::string msg = "")
{
    const char *colError = useColor ? "\033[1;31m" : "";
    const char *colWarning = useColor ? "\033[1;33m" : "";
    const char *colInfo = useColor ? "\033[1m" : "";
    const char *colDebug = useColor ? "\033[1m" : "";
    const char *colClear = useColor ? "\033[0m" : "";
    switch (l) {
    case logger::ERROR:
        if (msg == "") {
            msg = "ERROR-log-test";
        }
        return {std::format("{}{}{}{}\n", colError, "[ERROR] ", colClear, msg), msg};
    case logger::WARNING:
        if (msg == "") {
            msg = "WARNING-log-test";
        }
        return {std::format("{}{}{}{}\n", colWarning, "[WARN]  ", colClear, msg), msg};
    case logger::INFO:
        if (msg == "") {
            msg = "INFO-log-test";
        }
        return {std::format("{}{}{}{}\n", colInfo, "[INFO]  ", colClear, msg), msg};
    case logger::DEBUG:
        if (msg == "") {
            msg = "DEBUG-log-test";
        }
        return {std::format("{}{}{}{}\n", colDebug, "[DEBUG] ", colClear, msg), msg};
    case logger::SILENT:
        if (msg == "") {
            msg = "silence-is-absence";
        }
        return {std::string(""), msg};
    default:
        throw std::runtime_error("Unexpected logger level");
    }
}

TEST_CASE("Test setting log level via env. variable")
{
    using logger::log;
    using logger::LogSettings;

    // Remember values so that they can be restored after the tests
    const char *envLevlBefore = std::getenv("ROUGL_LOG_LEVEL");
    const char *envColBefore = std::getenv("ROUGL_LOG_COLOR");

    unsetenv("ROUGL_LOG_LEVEL");
    unsetenv("ROUGL_LOG_COLOR");
    const auto defaultSettings = logger::LogSettings();

    const auto levels = std::array{logger::SILENT, logger::ERROR, logger::WARNING, logger::INFO, logger::DEBUG};
    const auto levelStr = std::array{"Silent", "Error", "Warning", "Info", "Debug"};

    SECTION("Log level env. var parsing")
    {
        for (unsigned i = 0; i < levels.size(); i++) {
            REQUIRE(setenv("ROUGL_LOG_LEVEL", levelStr[i], 1) == 0);
            logger::settings = logger::LogSettings();
            REQUIRE(logger::settings.level == levels[i]);
        }

        logger::settings = defaultSettings;
        REQUIRE(setenv("ROUGL_LOG_COLOR", "On", 1) == 0);
        REQUIRE(setenv("ROUGL_LOG_LEVEL", "Invalid", 1) == 0);
        std::string sout, eout;
        {
            OutRedirect r;
            logger::settings = logger::LogSettings();
            sout = r.getCout();
            eout = r.getCerr();
        }
        auto [logExpected, msg] =
            formatLogLine(logger::ERROR, defaultSettings.useColor, "Invalid environment loggin level: 'Invalid'");
        REQUIRE(sout == std::string(""));
        REQUIRE(eout == logExpected);
    }

    SECTION("Log color env. var parsing")
    {
        REQUIRE(setenv("ROUGL_LOG_COLOR", "On", 1) == 0);
        logger::settings = logger::LogSettings();
        REQUIRE(logger::settings.useColor == true);

        REQUIRE(setenv("ROUGL_LOG_COLOR", "Off", 1) == 0);
        logger::settings = logger::LogSettings();
        REQUIRE(logger::settings.useColor == false);

        logger::settings = defaultSettings;
        REQUIRE(setenv("ROUGL_LOG_COLOR", "Invalid", 1) == 0);
        REQUIRE(setenv("ROUGL_LOG_LEVEL", "Error", 1) == 0);
        std::string sout, eout;
        {
            OutRedirect r;
            logger::settings = logger::LogSettings();
            sout = r.getCout();
            eout = r.getCerr();
        }
        auto [logExpected, msg] =
            formatLogLine(logger::ERROR, defaultSettings.useColor, "Invalid environment loggin color: 'Invalid'");
        REQUIRE(sout == std::string(""));
        REQUIRE(eout == logExpected);
    }

    SECTION("Printing different log levels")
    {
        for (bool useColor : {false, true}) {
            for (unsigned i = 0; i < levels.size(); i++) {
                REQUIRE(setenv("ROUGL_LOG_LEVEL", levelStr[i], 1) == 0);
                REQUIRE(setenv("ROUGL_LOG_COLOR", useColor ? "On" : "Off", 1) == 0);
                logger::settings = logger::LogSettings();

                for (unsigned j = 0; j < levels.size(); j++) {
                    auto [logExpected, msg] = formatLogLine(levels[j], useColor);
                    std::string sout, eout;
                    {
                        OutRedirect r;
                        logger::log(levels[j], msg);
                        sout = r.getCout();
                        eout = r.getCerr();
                    }
                    INFO(std::format("Test env level: {}, logging with level: {}, using color: {}\n", levelStr[i],
                                     levelStr[j], useColor ? "true" : "false"));
                    if (j > i || levels[j] == logger::SILENT) {
                        REQUIRE(sout == std::string(""));
                        REQUIRE(eout == std::string(""));
                    } else if (levels[j] == logger::ERROR) {
                        REQUIRE(sout == std::string(""));
                        REQUIRE(eout == logExpected);
                    } else {
                        REQUIRE(sout == logExpected);
                        REQUIRE(eout == std::string(""));
                    }
                }
            }
        }
    }

    // Restore logging settings to what they were before the tests
    {
        unsetenv("ROUGL_LOG_LEVEL");
        unsetenv("ROUGL_LOG_COLOR");
        if (envLevlBefore) {
            setenv("ROUGL_LOG_LEVEL", envLevlBefore, 1);
        }
        if (envColBefore) {
            setenv("ROUGL_LOG_COLOR", envColBefore, 1);
        }
        logger::settings = logger::LogSettings();
    }
}
#endif