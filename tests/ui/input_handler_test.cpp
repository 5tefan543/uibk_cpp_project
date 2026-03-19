#include "ui/input_handler.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("InputHandler can be constructed")
{
	REQUIRE_NOTHROW(ui::InputHandler{});
}

TEST_CASE("InputHandler pollInput is intentionally not exercised in unit tests yet")
{
	SUCCEED("pollInput depends on a live sf::RenderWindow and real OS event processing. Add an integration test later "
	        "if needed.");
}
