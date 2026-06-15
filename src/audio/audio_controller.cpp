#include "audio/audio_controller.hpp"
#include "logging/log.hpp"

#include <algorithm>
#include <exception>
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
        throw std::runtime_error("Failed to load music: " + file);
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

void AudioController::safePlaySound(const std::string &file)
{
    try {
        playSound(file);
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("AudioController sound playback failed: ") + e.what());
    }
}

void AudioController::safePlayMusic(const std::string &file)
{
    try {
        playMusic(file);
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("AudioController music playback failed: ") + e.what());
    }
}

void AudioController::safeStopMusic()
{
    try {
        stopMusic();
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("AudioController stop music failed: ") + e.what());
    }
}

void AudioController::safePauseMusic()
{
    try {
        pauseMusic();
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("AudioController pause music failed: ") + e.what());
    }
}

void AudioController::safeResumeMusic()
{
    try {
        resumeMusic();
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("AudioController resume music failed: ") + e.what());
    }
}

} // namespace audio
