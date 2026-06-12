#pragma once
#include "game/persisted_game.hpp"
#include "game_config.hpp"
#include "logging/log.hpp"
#include <glaze/glaze.hpp>

template <>
struct glz::meta<game::CharacterType> {
    static constexpr auto value =
        glz::enumerate("Melee", game::CharacterType::Melee, "Ranged", game::CharacterType::Ranged);
};

template <>
struct glz::meta<game::DamageKind> {
    static constexpr auto value =
        glz::enumerate("Projectile", game::DamageKind::Projectile, "MeleeArc", game::DamageKind::MeleeArc, "Beam",
                       game::DamageKind::Beam, "Area", game::DamageKind::Area);
};

template <>
struct glz::meta<game::PlayerStats> {
    static constexpr auto value =
        glz::object("maxHealth", &game::PlayerStats::maxHealth, "health", &game::PlayerStats::health, "attackPower",
                    &game::PlayerStats::attackPower, "attackSpeed", &game::PlayerStats::attackSpeed, "defense",
                    &game::PlayerStats::defense, "moveSpeed", &game::PlayerStats::moveSpeed, "speedOfAttack",
                    &game::PlayerStats::speedOfAttack, "attackRange", &game::PlayerStats::attackRange, "hasDash",
                    &game::PlayerStats::hasDash, "enemiesPierced", &game::PlayerStats::enemiesPierced, "score",
                    &game::PlayerStats::score, "currency", &game::PlayerStats::currency, "characterType",
                    &game::PlayerStats::characterType);
};

template <>
struct glz::meta<logger::LogLevel> {
    static constexpr auto value =
        glz::enumerate("Silent", logger::LogLevel::SILENT, "Error", logger::LogLevel::ERROR, "Warning",
                       logger::LogLevel::WARNING, "Info", logger::LogLevel::INFO, "Debug", logger::LogLevel::DEBUG);
};