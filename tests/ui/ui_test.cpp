#include "ui/ui.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("UI header is usable")
{
	SUCCEED("ui.hpp is included successfully. UI construction is not forced here because it opens an SFML window.");
}
