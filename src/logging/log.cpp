#include "logging/log.hpp"

#include <iostream>

namespace logger {

LogSettings::LogSettings(LogLevel level, bool useColor) : level(level), useColor(useColor) {}

bool LogSettings::shouldLog(LogLevel messageLevel) const
{
    return messageLevel != LogLevel::SILENT && messageLevel <= level;
}

const char *LogSettings::getLogLevelColor(LogLevel messageLevel) const
{
    if (!useColor) {
        return "";
    }

    switch (messageLevel) {
    case LogLevel::ERROR:
        return "\033[1;31m";
    case LogLevel::WARNING:
        return "\033[1;33m";
    case LogLevel::INFO:
    case LogLevel::DEBUG:
        return "\033[1m";
    case LogLevel::SILENT:
        return "";
    }

    return "";
}

const char *LogSettings::getLogLevelLabel(LogLevel messageLevel) const
{
    switch (messageLevel) {
    case LogLevel::ERROR:
        return "[ERROR] ";
    case LogLevel::WARNING:
        return "[WARN]  ";
    case LogLevel::INFO:
        return "[INFO]  ";
    case LogLevel::DEBUG:
        return "[DEBUG] ";
    case LogLevel::SILENT:
        return "";
    }

    return "";
}

const char *LogSettings::getClearColor() const
{
    return useColor ? "\033[0m" : "";
}

static LogSettings settings;

void configure(LogSettings newSettings)
{
    settings = newSettings;
}

void log(LogLevel level, const std::string &msg)
{
    if (!settings.shouldLog(level)) {
        return;
    }

    auto &out = level == LogLevel::ERROR ? std::cerr : std::cout;

    out << settings.getLogLevelColor(level) << settings.getLogLevelLabel(level) << settings.getClearColor() << msg
        << '\n';
}

} // namespace logger