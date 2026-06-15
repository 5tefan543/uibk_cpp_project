#pragma once

#include <SFML/Audio/SoundBuffer.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace audio {
class AudioCache {
  public:
    std::shared_ptr<sf::SoundBuffer> getBuffer(const std::string &filename)
    {
        if (auto it = buffers_.find(filename); it != buffers_.end()) {
            return it->second;
        }

        auto buffer = std::make_shared<sf::SoundBuffer>();

        if (!buffer->loadFromFile(filename)) {
            throw std::runtime_error("Failed to load: " + filename);
        }

        buffers_[filename] = buffer;
        return buffer;
    }

  private:
    std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> buffers_;
};
} // namespace audio
