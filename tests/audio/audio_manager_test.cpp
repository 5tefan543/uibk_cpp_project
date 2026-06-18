#include "audio/audio_controller.hpp"
#include "shared/audio_test_util.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

// ---------------------------------------------------------------------------
// AudioCache – error paths
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController playSound throws for missing file")
{
    
    audio::AudioController controller;

    REQUIRE_THROWS(controller.playSound("this/path/does/not/exist.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playMusic does not throw for missing file")
{
    
    audio::AudioController controller;

    REQUIRE_NOTHROW(controller.playMusic("this/path/does/not/exist.ogg"));
}


// ---------------------------------------------------------------------------
// AudioController – positive sound path  (audio_controller.cpp lines 15-20)
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController playSound with valid file does not throw")
{
    test::writeMinimalWav("fixture.wav");
    audio::AudioController controller;

    REQUIRE_NOTHROW(controller.playSound("fixture.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController update does not throw after valid playSound")
{
    test::writeMinimalWav("fixture.wav");
    audio::AudioController controller;

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
    audio::AudioController controller;

    REQUIRE_NOTHROW(controller.playMusic("music.wav"));
}

TEST_CASE_METHOD(TestFixture, "AudioController playMusic replaces currently playing music without throwing")
{
    test::writeMinimalWav("music.wav");
    audio::AudioController controller;

    controller.playMusic("music.wav");
    // Second call must stop the first stream before opening the new one
    REQUIRE_NOTHROW(controller.playMusic("music.wav"));
}

// ---------------------------------------------------------------------------
// AudioController – conditional music controls
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(TestFixture, "AudioController stopMusic does not throw when no music is loaded")
{
    audio::AudioController controller;

    REQUIRE_NOTHROW(controller.stopMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController stopMusic does not throw after music has been started")
{
    test::writeMinimalWav("music.wav");
    audio::AudioController controller;

    controller.playMusic("music.wav");
    REQUIRE_NOTHROW(controller.stopMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController pauseMusic is a no-op when music is not playing")
{
    audio::AudioController controller;

    // status is Stopped → guard skips pause
    REQUIRE_NOTHROW(controller.pauseMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController pauseMusic pauses music that is currently playing")
{
    // Covers the status == Playing → music_.pause() branch
    test::writeMinimalWav("music.wav");
    audio::AudioController controller;

    controller.playMusic("music.wav");
    REQUIRE_NOTHROW(controller.pauseMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController resumeMusic is a no-op when music is not paused")
{
    
    audio::AudioController controller;

    // status is Stopped → guard skips resume
    REQUIRE_NOTHROW(controller.resumeMusic());
}

TEST_CASE_METHOD(TestFixture, "AudioController resumeMusic resumes music that is currently paused")
{
    // Covers the status == Paused → music_.play() branch
    test::writeMinimalWav("music.wav");
    audio::AudioController controller;

    controller.playMusic("music.wav");
    controller.pauseMusic();
    REQUIRE_NOTHROW(controller.resumeMusic());
}
