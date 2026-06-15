#pragma once

#include "audio_cache.hpp"
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

namespace audio {
class AudioController {
  public:
    explicit AudioController(AudioCache &cache) : cache_(cache) {}

    void playSound(const std::string &file)
    {
        auto buffer = cache_.getBuffer(file);

        auto sound = std::make_unique<sf::Sound>(*buffer);

        sound->play();

        activeSounds_.push_back({std::move(sound), std::move(buffer)});
    }

    void playMusic(const std::string &file)
    {
        music_.stop();

        if (!music_.openFromFile(file)) {
            throw std::runtime_error("Failed to load music: " + file);
        }

        music_.setLooping(true);
        music_.play();
    }

    void stopMusic() { music_.stop(); }

    void update()
    {
        std::erase_if(activeSounds_,
                      [](const ActiveSound &s) { return s.sound->getStatus() == sf::Sound::Status::Stopped; });
    }

  private:
    struct ActiveSound {
        std::unique_ptr<sf::Sound> sound;
        std::shared_ptr<sf::SoundBuffer> buffer;
    };

    AudioCache &cache_;

    std::vector<ActiveSound> activeSounds_;

    sf::Music music_;
};
} // namespace audio