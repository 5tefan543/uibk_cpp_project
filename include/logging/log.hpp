#pragma once
#include <string>

namespace logger {

constexpr const char *LOG_LEVEL_ENV_VAR = "ROUGL_LOG_LEVEL";
constexpr const char *LOG_COLOR_ENV_VAR = "ROUGL_LOG_COLOR";

// Log levels in increasing order of verbosity: SILENT < ERROR < WARNING < INFO < DEBUG
// Enum order is important for level hierachy comparison. Higher verbosity (higher enum value) includes all lower
// verbosity (lower enum values <-> more severe) levels. SILENT guarantes no logging output.
enum LogLevel { SILENT, ERROR, WARNING, INFO, DEBUG };

class LogSettings {
    LogLevel level_;
    bool useColor_;

  public:
    LogSettings(LogLevel level = LogLevel::ERROR, bool useColor = true);
    LogLevel getLevel() const;
    bool getUseColor() const;
    bool shouldLog(LogLevel messageLevel) const;
    const char *getLogLevelColor(LogLevel messageLevel) const;
    const char *getClearColor() const;
    const char *getLogLevelLabel(LogLevel messageLevel) const;
};

inline LogLevel LogSettings::getLevel() const
{
    return level_;
}

inline bool LogSettings::getUseColor() const
{
    return useColor_;
}

void configure(LogLevel level = LogLevel::ERROR, bool useColor = true);
void log(LogLevel level, const std::string &msg);

} // namespace logger