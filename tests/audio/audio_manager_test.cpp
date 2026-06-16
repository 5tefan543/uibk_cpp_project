#include "audio/audio_cache.hpp"
#include "audio/audio_controller.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

// Writes a minimal valid mono 16-bit 44100 Hz silent PCM WAV file so tests
// can exercise positive audio paths without shipping binary assets.
void writeMinimalWav(const std::filesystem::path &path)
{
    constexpr uint32_t sampleRate = 44100;
    constexpr uint16_t numChannels = 1;
    constexpr uint16_t bitsPerSample = 16;
    constexpr uint32_t numSamples = 4;
    constexpr uint32_t dataSize = numSamples * numChannels * (bitsPerSample / 8U);
    constexpr uint32_t riffSize = 36 + dataSize;
    constexpr uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8U);
    constexpr uint16_t blockAlign = numChannels * (bitsPerSample / 8U);
    constexpr uint32_t fmtChunkSize = 16;
    constexpr uint16_t pcmFormat = 1;

    std::ofstream out(path, std::ios::binary);
    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char *>(&riffSize), sizeof(riffSize));
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char *>(&fmtChunkSize), sizeof(fmtChunkSize));
    out.write(reinterpret_cast<const char *>(&pcmFormat), sizeof(pcmFormat));
    out.write(reinterpret_cast<const char *>(&numChannels), sizeof(numChannels));
    out.write(reinterpret_cast<const char *>(&sampleRate), sizeof(sampleRate));
    out.write(reinterpret_cast<const char *>(&byteRate), sizeof(byteRate));
    out.write(reinterpret_cast<const char *>(&blockAlign), sizeof(blockAlign));
    out.write(reinterpret_cast<const char *>(&bitsPerSample), sizeof(bitsPerSample));
    out.write("data", 4);
    out.write(reinterpret_cast<const char *>(&dataSize), sizeof(dataSize));
    const std::vector<uint8_t> silence(dataSize, 0);
    out.write(reinterpret_cast<const char *>(silence.data()), static_cast<std::streamsize>(dataSize));
}

} // namespace

// ---------------------------------------------------------------------------
// AudioCache – error paths
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer throws for missing file")
{
    audio::AudioCache cache;

    REQUIRE_THROWS(cache.getBuffer("this/path/does/not/exist.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playSound throws for missing file")
{
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    REQUIRE_THROWS(controller.playSound("this/path/does/not/exist.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playMusic throws for missing file")
{
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    REQUIRE_THROWS(controller.playMusic("this/path/does/not/exist.ogg"));
}

TEST_CASE_METHOD(TestFixture, "AudioController safe audio methods swallow failures")
{
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    REQUIRE_NOTHROW(controller.safePlaySound("this/path/does/not/exist.wav"));
    REQUIRE_NOTHROW(controller.safePlayMusic("this/path/does/not/exist.ogg"));
    REQUIRE_NOTHROW(controller.safePauseMusic());
    REQUIRE_NOTHROW(controller.safeResumeMusic());
    REQUIRE_NOTHROW(controller.safeStopMusic());
    REQUIRE_NOTHROW(controller.update());
}

// ---------------------------------------------------------------------------
// AudioCache – caching / deduplication  (audio_cache.hpp lines 13-24)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer returns the same shared_ptr for the same file")
{
    writeMinimalWav("fixture.wav");
    audio::AudioCache cache;

    auto buf1 = cache.getBuffer("fixture.wav");
    auto buf2 = cache.getBuffer("fixture.wav");

    REQUIRE(buf1 != nullptr);
    REQUIRE(buf1.get() == buf2.get()); // identical pointer — no reload happened
}

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer returns different buffers for different files")
{
    writeMinimalWav("a.wav");
    writeMinimalWav("b.wav");
    audio::AudioCache cache;

    auto bufA = cache.getBuffer("a.wav");
    auto bufB = cache.getBuffer("b.wav");

    REQUIRE(bufA != nullptr);
    REQUIRE(bufB != nullptr);
    REQUIRE(bufA.get() != bufB.get());
}

TEST_CASE_METHOD(TestFixture, "AudioCache keeps buffer alive as long as cache holds it")
{
    writeMinimalWav("fixture.wav");
    audio::AudioCache cache;

    std::weak_ptr<sf::SoundBuffer> weak;
    {
        auto buf = cache.getBuffer("fixture.wav");
        weak = buf;
    } // local shared_ptr is released; cache still owns it

    REQUIRE_FALSE(weak.expired());
}

// ---------------------------------------------------------------------------
// AudioController – positive sound path  (audio_controller.cpp lines 15-20)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController playSound with valid file does not throw")
{
    writeMinimalWav("fixture.wav");
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    REQUIRE_NOTHROW(controller.playSound("fixture.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController update does not throw after valid playSound")
{
    writeMinimalWav("fixture.wav");
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    controller.playSound("fixture.wav");
    REQUIRE_NOTHROW(controller.update());
}

// ---------------------------------------------------------------------------
// AudioController – conditional music controls  (audio_controller.cpp lines 41-48)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController stopMusic does not throw when no music is loaded")
{
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    REQUIRE_NOTHROW(controller.stopMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController pauseMusic is a no-op when music is not playing")
{
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    // Without a loaded music track, status is Stopped → the guard skips pause.
    REQUIRE_NOTHROW(controller.pauseMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController resumeMusic is a no-op when music is not paused")
{
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    // Without a loaded or paused music track, status is Stopped → the guard skips resume.
    REQUIRE_NOTHROW(controller.resumeMusic());
}
