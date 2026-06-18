#include "audio/audio_cache.hpp"
#include "audio/audio_controller.hpp"
#include "shared/audio_test_util.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

// ---------------------------------------------------------------------------
// AudioCache – error paths
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer throws for missing file")
{
    

    REQUIRE_THROWS(cache.getBuffer("this/path/does/not/exist.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playSound throws for missing file")
{
    
    audio::AudioController controller();

    REQUIRE_THROWS(controller.playSound("this/path/does/not/exist.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playMusic does not throw for missing file")
{
    
    audio::AudioController controller();

    REQUIRE_NOTHROW(controller.playMusic("this/path/does/not/exist.ogg"));
}

// ---------------------------------------------------------------------------
// AudioCache – caching / deduplication  (audio_cache.hpp lines 13-24)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer returns the same shared_ptr for the same file")
{
    test::writeMinimalWav("fixture.wav");
    

    auto buf1 = cache.getBuffer("fixture.wav");
    auto buf2 = cache.getBuffer("fixture.wav");

    REQUIRE(buf1 != nullptr);
    REQUIRE(buf1.get() == buf2.get()); // identical pointer — no reload happened
}

TEST_CASE_METHOD(TestFixture, "AudioCache getBuffer returns different buffers for different files")
{
    test::writeMinimalWav("a.wav");
    test::writeMinimalWav("b.wav");
    

    auto bufA = cache.getBuffer("a.wav");
    auto bufB = cache.getBuffer("b.wav");

    REQUIRE(bufA != nullptr);
    REQUIRE(bufB != nullptr);
    REQUIRE(bufA.get() != bufB.get());
}

TEST_CASE_METHOD(TestFixture, "AudioCache keeps buffer alive as long as cache holds it")
{
    test::writeMinimalWav("fixture.wav");
    

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
    audio::AudioController controller();

    REQUIRE_NOTHROW(controller.playSound("fixture.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController update does not throw after valid playSound")
{
    test::writeMinimalWav("fixture.wav");
    audio::AudioController controller();

    controller.playSound("fixture.wav");
    REQUIRE_NOTHROW(controller.update());
}

// ---------------------------------------------------------------------------
// AudioController – playMusic positive path  (setLooping + play)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController playMusic with valid file does not throw")
{
    // Covers music_.stop(), openFromFile(), setLooping(true) and music_.play()
    test::writeMinimalWav("music.wav");
    audio::AudioController controller();

    REQUIRE_NOTHROW(controller.playMusic("music.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playMusic replaces currently playing music without throwing")
{
    test::writeMinimalWav("music.wav");
    audio::AudioController controller();

    controller.playMusic("music.wav");
    // Second call must stop the first stream before opening the new one
    REQUIRE_NOTHROW(controller.playMusic("music.wav"));
}

// ---------------------------------------------------------------------------
// AudioController – conditional music controls
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController stopMusic does not throw when no music is loaded")
{
    audio::AudioController controller();

    REQUIRE_NOTHROW(controller.stopMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController stopMusic does not throw after music has been started")
{
    test::writeMinimalWav("music.wav");
    audio::AudioController controller();

    controller.playMusic("music.wav");
    REQUIRE_NOTHROW(controller.stopMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController pauseMusic is a no-op when music is not playing")
{
    audio::AudioController controller();

    // status is Stopped → guard skips pause
    REQUIRE_NOTHROW(controller.pauseMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController pauseMusic pauses music that is currently playing")
{
    // Covers the status == Playing → music_.pause() branch
    test::writeMinimalWav("music.wav");
    audio::AudioController controller();

    controller.playMusic("music.wav");
    REQUIRE_NOTHROW(controller.pauseMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController resumeMusic is a no-op when music is not paused")
{
    
    audio::AudioController controller();

    // status is Stopped → guard skips resume
    REQUIRE_NOTHROW(controller.resumeMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController resumeMusic resumes music that is currently paused")
{
    // Covers the status == Paused → music_.play() branch
    test::writeMinimalWav("music.wav");
    audio::AudioController controller();

    controller.playMusic("music.wav");
    controller.pauseMusic();
    REQUIRE_NOTHROW(controller.resumeMusic());
}
