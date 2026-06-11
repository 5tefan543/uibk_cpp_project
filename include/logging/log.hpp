#pragma once
#include <string>

namespace logger {

// Enum order is important for level hierachy comparison. Higher verbosity (higher enum value) includes all lower
// verbosity (lower enum values <-> more severe) levels. SILENT guarantes no logging output.
//
// Verbosity high to low: DEBUG > INFO > WARNING > ERROR > SILENT
enum Level { SILENT, ERROR, WARNING, INFO, DEBUG };

void log(Level l, const std::string &msg);

} // namespace logger