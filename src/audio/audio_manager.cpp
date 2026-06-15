#include "audio/audio_manager.hpp"
#include "logging/log.hpp"
#include <filesystem>
#include <fstream>
#include <format>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace audio {

AudioManager::~AudioManager() = default;

void AudioManager::setMusicVolume(float volume)
{
    musicVolume_ = volume;
    if (music_) music_->setVolume(volume);
}


bool AudioManager::loadMusic(const std::string &name, const std::filesystem::path &path)
{
    const auto size = std::filesystem::file_size(path);

    musicFileData_ = std::vector<std::byte>(size);

    std::ifstream file(path, std::ios::binary);
    file.read(reinterpret_cast<char*>(musicFileData_.data()),
              static_cast<std::streamsize>(size));

    return true;
}

void AudioManager::startBackGroundMusic(const std::string &name) {
    if(name == songName_){
        if (music_) {
            music_->stop();
            music_->setLooping(true);
            music_->play();
        }
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
    music_ = std::make_unique<sf::Music>();
    if(!music_->openFromMemory(musicFileData_.data(), musicFileData_.size())){
        //error could not load music from memory
    }
    music_->setVolume(musicVolume_);
    music_->setLooping(true);
    music_->play();
}

void AudioManager::stopBackGroundMusic() {
    if (music_) music_->stop();
    musicFileData_.clear();    
}

bool AudioManager::loadSound(const std::string &name, const std::filesystem::path &path)
{
    auto buffer = std::make_unique<sf::SoundBuffer>();

    if (!buffer->loadFromFile(path)) {
        // logger::log(logger::ERROR, std::format("Soundfile {} not found", path.string()).c_str());
        return false;
    }

    soundCache_.insert(std::pair<std::string, std::unique_ptr<sf::SoundBuffer>>(name, std::move(buffer)));
    return true;
}

void AudioManager::playSound(const std::string &name)
{

    if (!soundCache_.contains(name)) {
        // logger::log(logger::DEBUG, "Soundfile not in cache");
        // TODO get path from AssetManager via name
        const std::filesystem::path path = "assets/audio/effects/menu_move.wav";
        loadSound(name, path);
    }

    auto it = soundCache_.find(name);
    if (it == soundCache_.end()) return;
    sf::Sound sound(*it->second);
    sound.setLooping(false);
    sound.setVolume(soundVolume_);
    sound.play();
}

} // namespace audio