#include "audio/audio_controller.hpp"

#include "logging/log.hpp"
#include <algorithm>
#include <format>
#include <stdexcept>
#include <string>

namespace audio {

AudioController::AudioController(AudioCache &cache) : cache_(cache) {}

void AudioController::playSound(const std::string &file)
{
    auto buffer = cache_.getBuffer(file);
    auto sound = std::make_unique<sf::Sound>(*buffer);

    sound->play();
    activeSounds_.push_back({std::move(sound), std::move(buffer)});
}

void AudioController::playMusic(const std::string &file)
{
    music_.stop();

    if (!music_.openFromFile(file)) {
        logger::log(logger::LogLevel::ERROR, std::format("Failed to load music: {}", file));
        return;
    }

    music_.setLooping(true);
    music_.play();
}

void AudioController::stopMusic()
{
    music_.stop();
}

void AudioController::pauseMusic()
{
    if (music_.getStatus() == sf::SoundSource::Status::Playing) {
        music_.pause();
    }
}

void AudioController::resumeMusic()
{
    if (music_.getStatus() == sf::SoundSource::Status::Paused) {
        music_.play();
    }
}

void AudioController::update()
{
    std::erase_if(activeSounds_, [](const ActiveSound &activeSound) {
        return activeSound.sound->getStatus() == sf::Sound::Status::Stopped;
    });
}

} // namespace audio
