#include "audio/audio_manager.hpp"
#include "logging/log.hpp"
#include <filesystem>
#include <format>

namespace audio {

void AudioManager::startBackGroundMusic(const std::string &name) {};
void AudioManager::stopBackGroundMusic() {};

bool AudioManager::loadSound(const std::string &name, const std::filesystem::path &path)
{
    sf::SoundBuffer buffer;

    if (!buffer.loadFromFile(path)) {
        // logger::log(logger::ERROR, std::format("Soundfile {} not found", path.string()).c_str());
        return false;
    }

    
    soundCache_.insert(std::pair<std::string, sf::SoundBuffer>(name, buffer));
    return true;
};

void AudioManager::playSound(const std::string &name)
{

    if (!soundCache_.contains(name)) {
        // logger::log(logger::DEBUG, "Soundfile not in cache");
        // TODO get path from AssetManager via name
        const std::filesystem::path &path = "assets/audio/effects/menu_move.wav";
        loadSound(name, path);
    }

    auto buffer = soundCache_.find("name")->second;
    sf::Sound sound(buffer);
    sound.setLooping(false);
    sound.setVolume(soundVolume_);
    sound.play();
}

bool AudioManager::loadMusic(const std::string &name, const std::filesystem::path &path)
{
    return true;
};
} // namespace audio