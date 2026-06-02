#pragma once

#include <string>

namespace logger {

// Log levels in increasing order of verbosity: SILENT < ERROR < WARNING < INFO < DEBUG
enum LogLevel { SILENT, ERROR, WARNING, INFO, DEBUG };

class LogSettings {
    LogLevel level_ = LogLevel::ERROR;
    bool useColor_ = true;

  public:
    LogSettings(LogLevel level = LogLevel::ERROR, bool useColor = true);
    bool shouldLog(LogLevel messageLevel) const;
    const char *getLogLevelColor(LogLevel messageLevel) const;
    const char *getLogLevelLabel(LogLevel messageLevel) const;
    const char *getClearColor() const;
};

void configure(LogSettings settings);
void log(LogLevel level, const std::string &msg);

} // namespace logger