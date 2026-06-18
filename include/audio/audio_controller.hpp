#pragma once

#include "audio/audio_cache.hpp"
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <memory>
#include <string>
#include <vector>

namespace audio {
class AudioController {
  public:

    void playSound(const std::string &file);
    void playMusic(const std::string &file);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void update();

  private:
    struct ActiveSound {
        std::unique_ptr<sf::Sound> sound;
        std::shared_ptr<sf::SoundBuffer> buffer;
    };

    AudioCache cache_;

    std::vector<ActiveSound> activeSounds_;

    sf::Music music_;
};
} // namespace audio