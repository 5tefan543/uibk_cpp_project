#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <vector>

namespace audio {

class AudioManager {
  private:
    float soundVolume_ = 100.f;
    float musicVolume_ = 100.f;
    std::map<std::string, sf::SoundBuffer> soundCache_;
    std::vector<std::byte> musicFileData_;
    std::string songName_;
    sf::Music music_;
    bool loadMusic(const std::string &name, const std::filesystem::path &path);
    bool loadSound(const std::string &name, const std::filesystem::path &path);

  public:
    void playSound(const std::string &name);
    void startBackGroundMusic(const std::string &name);
    void stopBackGroundMusic();


    float getSoundVolume() const { return soundVolume_; };
    void setSoundVolume(float volume) { soundVolume_ = volume;};
    float getMusicVolume() const { return musicVolume_; };
    void setMusicVolume(float volume) { musicVolume_ = volume; music_.setVolume(volume); };
};

} // namespace audio