#include <SFML/Audio.hpp>
#include <map>
#include <string>

namespace audio {

class AudioManager {
  private:
    float soundVolume_ = 1.0f;
    float musicVolume_ = 1.0f;
    std::map<std::string, sf::Sound> soundCache_;
    std::map<std::string, sf::Music> musicCache_;

  public:
    AudioManager();
    ~AudioManager();

    void playSound();
    void startBackGroundMusic();
    void stopBackGroundMusic();
};

} // namespace audio