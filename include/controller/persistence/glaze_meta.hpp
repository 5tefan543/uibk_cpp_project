#pragma once
#include "game/ecs/components/animation.hpp"
#include "game/persisted_game.hpp"
#include "game/store/store_item.hpp"
#include "logging/log.hpp"
#include "view/font.hpp"
#include <glaze/glaze.hpp>

template <>
struct glz::meta<view::FontType> {
    static constexpr auto value = glz::enumerate("Default", view::FontType::Default);
};

template <>
struct glz::meta<game::CharacterType> {
    static constexpr auto value =
        glz::enumerate("Melee", game::CharacterType::Melee, "Ranged", game::CharacterType::Ranged);
};

template <>
struct glz::meta<game::EnemyType> {
    static constexpr auto value = glz::enumerate("Blob", game::EnemyType::Blob, "Boss", game::EnemyType::Boss);
};

template <>
struct glz::meta<game::DamageKind> {
    static constexpr auto value =
        glz::enumerate("Projectile", game::DamageKind::Projectile, "Unicorn", game::DamageKind::Unicorn, "MeleeArc",
                       game::DamageKind::MeleeArc, "Beam", game::DamageKind::Beam, "Area", game::DamageKind::Area);
};

template <>
struct glz::meta<game::PlayerStats> {
    static constexpr auto value = glz::object(
        "maxHealth", &game::PlayerStats::maxHealth, "health", &game::PlayerStats::health, "attackPower",
        &game::PlayerStats::attackPower, "attackSpeed", &game::PlayerStats::attackSpeed, "specialAttackSpeed",
        &game::PlayerStats::specialAttackSpeed, "defense", &game::PlayerStats::defense, "moveSpeed",
        &game::PlayerStats::moveSpeed, "speedOfAttack", &game::PlayerStats::speedOfAttack, "attackRange",
        &game::PlayerStats::attackRange, "hasDash", &game::PlayerStats::hasDash, "enemiesPierced",
        &game::PlayerStats::enemiesPierced, "score", &game::PlayerStats::score, "currency",
        &game::PlayerStats::currency, "characterType", &game::PlayerStats::characterType);
};

template <>
struct glz::meta<logger::LogLevel> {
    static constexpr auto value =
        glz::enumerate("Silent", logger::LogLevel::SILENT, "Error", logger::LogLevel::ERROR, "Warning",
                       logger::LogLevel::WARNING, "Info", logger::LogLevel::INFO, "Debug", logger::LogLevel::DEBUG);
};

template <>
struct glz::meta<game::AnimationDirection> {
    static constexpr auto value = glz::enumerate(
        "None", game::AnimationDirection::None, "Left", game::AnimationDirection::Left, "Right",
        game::AnimationDirection::Right, "Up", game::AnimationDirection::Up, "Down", game::AnimationDirection::Down);
};

template <>
struct glz::meta<game::AnimationState> {
    static constexpr auto value = glz::enumerate("Idle", game::AnimationState::Idle, "Walk", game::AnimationState::Walk,
                                                 "Attack", game::AnimationState::Attack, "Hit",
                                                 game::AnimationState::Hit, "Death", game::AnimationState::Death);
};

template <>
struct glz::meta<game::StoreItemType> {
    static constexpr auto value =
        glz::enumerate("Common", game::StoreItemType::Common, "Uncommon", game::StoreItemType::Uncommon, "Rare",
                       game::StoreItemType::Rare, "Epic", game::StoreItemType::Epic);
};