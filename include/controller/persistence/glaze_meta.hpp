#pragma once
#include "logging/log.hpp"
#include <glaze/glaze.hpp>

template <>
struct glz::meta<logger::LogLevel> {
    static constexpr auto value =
        glz::enumerate("Silent", logger::LogLevel::SILENT, "Error", logger::LogLevel::ERROR, "Warning",
                       logger::LogLevel::WARNING, "Info", logger::LogLevel::INFO, "Debug", logger::LogLevel::DEBUG);
};