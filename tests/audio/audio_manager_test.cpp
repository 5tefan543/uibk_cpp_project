#include "audio/audio_cache.hpp"
#include "audio/audio_controller.hpp"
#include "logging/log.hpp"
#include "shared/audio_test_util.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <sstream>

namespace {
struct StdoutCapture {
    StdoutCapture() : oldBuf_(std::cout.rdbuf(ss_.rdbuf())) {}
    ~StdoutCapture() { std::cout.rdbuf(oldBuf_); }
    std::string get() const { return ss_.str(); }

  private:
    std::stringstream ss_;
    std::streambuf *oldBuf_;
};
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
    test::writeMinimalWav("fixture.wav");
    audio::AudioCache cache;

    auto buf1 = cache.getBuffer("fixture.wav");
    auto buf2 = cache.getBuffer("fixture.wav");

    REQUIRE(buf1 != nullptr);
    REQUIRE(buf1.get() == buf2.get()); // identical pointer — no reload happened
}

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer returns different buffers for different files")
{
    test::writeMinimalWav("a.wav");
    test::writeMinimalWav("b.wav");
    audio::AudioCache cache;

    auto bufA = cache.getBuffer("a.wav");
    auto bufB = cache.getBuffer("b.wav");

    REQUIRE(bufA != nullptr);
    REQUIRE(bufB != nullptr);
    REQUIRE(bufA.get() != bufB.get());
}

TEST_CASE_METHOD(TestFixture, "AudioCache keeps buffer alive as long as cache holds it")
{
    test::writeMinimalWav("fixture.wav");
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
    test::writeMinimalWav("fixture.wav");
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    REQUIRE_NOTHROW(controller.playSound("fixture.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController update does not throw after valid playSound")
{
    test::writeMinimalWav("fixture.wav");
    audio::AudioCache cache;
    audio::AudioController controller(cache);

    controller.playSound("fixture.wav");
    REQUIRE_NOTHROW(controller.update());
}

// ---------------------------------------------------------------------------
// AudioController – safe method catch blocks log a warning
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController safePlaySound logs a warning when sound file is missing")
{
    logger::configure(logger::WARNING, false);
    StdoutCapture capture;

    audio::AudioCache cache;
    audio::AudioController controller(cache);
    controller.safePlaySound("no/such/file.wav");

    REQUIRE(capture.get().find("AudioController sound playback failed") != std::string::npos);
}

TEST_CASE_METHOD(TestFixture, "AudioController safePlayMusic logs a warning when music file is missing")
{
    logger::configure(logger::WARNING, false);
    StdoutCapture capture;

    audio::AudioCache cache;
    audio::AudioController controller(cache);
    controller.safePlayMusic("no/such/file.ogg");

    REQUIRE(capture.get().find("AudioController music playback failed") != std::string::npos);
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
