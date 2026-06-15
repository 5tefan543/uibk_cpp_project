#include "audio/audio_manager.hpp"
#include "logging/log.hpp"
#include <filesystem>
#include <fstream>
#include <format>

namespace audio {

bool AudioManager::loadMusic(const std::string &name, const std::filesystem::path &path)
{
    const auto size = std::filesystem::file_size(path);

    musicFileData_ = std::vector<std::byte>(size);

    std::ifstream file(path, std::ios::binary);
    file.read(reinterpret_cast<char*>(musicFileData_.data()),
              static_cast<std::streamsize>(size));

    return true;
};
void AudioManager::startBackGroundMusic(const std::string &name) {
    if(name == songName_){
        music_.stop();
        music_.setLooping(true);
        music_.play();
        return;
    }
    if(!musicFileData_.empty()){
        stopBackGroundMusic();
    }
    // get from config via asset manager and name
    auto path = std::filesystem::path(""); 

    if(!loadMusic(name, path)){
        // error could not open file
    }
    if(!music_.openFromMemory(musicFileData_.data(), musicFileData_.size())){
        //error could not load music from memory
    }
    music_.setVolume(musicVolume_);
    music_.setLooping(true);
    music_.play();
};

void AudioManager::stopBackGroundMusic() {
    music_.stop();
    musicFileData_.clear();    
};

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

} // namespace audio