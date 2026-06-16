#include "audio/audio_cache.hpp"
#include "audio/audio_controller.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

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
