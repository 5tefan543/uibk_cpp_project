#include "logging/log.hpp"
#include <format>
#include <iostream>
#include <optional>
#include <string>

namespace logger {

namespace {

std::optional<LogLevel> parseLogLevelEnv()
{
    if (const char *envLevel = std::getenv(logLevelEnvVar)) {
        const std::string value{envLevel};

        if (value == "Error") {
            return ERROR;
        }
        if (value == "Warning") {
            return WARNING;
        }
        if (value == "Info") {
            return INFO;
        }
        if (value == "Debug") {
            return DEBUG;
        }
        if (value == "Silent") {
            return SILENT;
        }

        std::cerr << "[ERROR] Invalid environment logging level: '" << value << "'\n";
        std::cerr << "Valid values are: Silent, Error, Warning, Info, Debug\n";
    }

    return std::nullopt;
}

std::optional<bool> parseLogColorEnv()
{
    if (const char *envColor = std::getenv(logColorEnvVar)) {
        const std::string value{envColor};

        if (value == "On") {
            return true;
        }
        if (value == "Off") {
            return false;
        }

        std::cerr << "[ERROR] Invalid environment logging color setting: '" << value << "'\n";
        std::cerr << "Valid values are: On, Off\n";
    }

    return std::nullopt;
}

} // namespace

LogSettings::LogSettings(LogLevel level, bool useColor) : level_(level), useColor_(useColor)
{
    if (auto envLevel = parseLogLevelEnv()) {
        level_ = envLevel.value();
    }

    if (auto envColor = parseLogColorEnv()) {
        useColor_ = envColor.value();
    }
}

bool LogSettings::shouldLog(LogLevel messageLevel) const
{
    return messageLevel != LogLevel::SILENT && messageLevel <= level_;
}

const char *LogSettings::getLogLevelColor(LogLevel messageLevel) const
{
    if (!useColor_) {
        return "";
    }

    switch (messageLevel) {
    case LogLevel::ERROR:
        return "\033[1;31m"; // bold red
    case LogLevel::WARNING:
        return "\033[1;33m"; // bold yellow
    case LogLevel::INFO:
        return "\033[1;32m"; // bold green
    case LogLevel::DEBUG:
        return "\033[1m"; // bold default
    default:
        return "";
    }
}

const char *LogSettings::getClearColor() const
{
    return useColor_ ? "\033[0m" : "";
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
    default:
        return "";
    }
}

static LogSettings settings;

void configure(LogLevel level, bool useColor)
{
    settings = LogSettings(level, useColor);
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