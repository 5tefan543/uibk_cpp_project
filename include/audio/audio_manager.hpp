#pragma once
#include <SFML/Audio.hpp>
#include <map>
#include <string>

namespace audio {

class AudioManager {
  private:
    float soundVolume_ = 100.f;
    float musicVolume_ = 100.f;
    std::map<std::string, sf::Sound> soundCache_;
    std::map<std::string, sf::Music> musicCache_;
    bool loadMusic(const std::string &name, const std::filesystem::path &path);

  public:
    bool loadSound(const std::string &name, const std::filesystem::path &path);
    void playSound(const std::string &name);
    void startBackGroundMusic(const std::string &name);
    void stopBackGroundMusic();
    float getSoundVolume() const { return soundVolume_; };
    void setSoundVolume(float volume) { soundVolume_ = volume; };
    float getMusicVolume() const { return musicVolume_; };
    void setMusicVolume(float volume) { musicVolume_ = volume; };
};

} // namespace audio