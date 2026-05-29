#include "logging/log.hpp"
#include <format>
#include <iostream>
#include <string>

namespace logger {

struct LogSettings {
    Level level = ERROR;
    bool useColor = true;

    LogSettings()
    {
        if (const char *envLevl = std::getenv("ROUGL_LOG_LEVEL")) {
            if (std::string("Error").compare(envLevl) == 0) {
                level = ERROR;
            } else if (std::string("Warning").compare(envLevl) == 0) {
                level = WARNING;
            } else if (std::string("Info").compare(envLevl) == 0) {
                level = INFO;
            } else if (std::string("Debug").compare(envLevl) == 0) {
                level = DEBUG;
            } else if (std::string("Silent").compare(envLevl) == 0) {
                level = SILENT;
            } else {
                log(ERROR, std::format("Invalid environment loggin level: '{}'", envLevl));
            }
        }

        if (const char *envColor = std::getenv("ROUGL_LOG_COLOR")) {
            if (std::string("On").compare(envColor) == 0) {
                useColor = true;
            } else if (std::string("Off").compare(envColor) == 0) {
                useColor = false;
            } else {
                log(ERROR, std::format("Invalid environment loggin color: '{}'", envColor));
            }
        }
    }
};

static LogSettings settings;

void log(Level l, const std::string &msg)
{
    const char *colError = settings.useColor ? "\033[1;31m" : "";
    const char *colWarning = settings.useColor ? "\033[1;33m" : "";
    const char *colInfo = settings.useColor ? "\033[1m" : "";
    const char *colDebug = settings.useColor ? "\033[1m" : "";
    const char *colClear = "\033[0m";

    if (l <= settings.level) {
        switch (l) {
        case ERROR:
            std::cerr << colError << "[ERROR] " << colClear << msg << std::endl;
            break;
        case WARNING:
            std::cout << colWarning << "[WARN]  " << colClear << msg << std::endl;
            break;
        case INFO:
            std::cout << colInfo << "[INFO]  " << colClear << msg << std::endl;
            break;
        case DEBUG:
            std::cout << colDebug << "[DEBUG] " << colClear << msg << std::endl;
            break;
        case SILENT:
            break;
        }
    }
}

} // namespace logger