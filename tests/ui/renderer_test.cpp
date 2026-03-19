#include "ui/renderer.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Renderer can be constructed")
{
	REQUIRE_NOTHROW(ui::Renderer{});
}

TEST_CASE("Renderer render is intentionally not exercised in unit tests yet")
{
	SUCCEED("render depends on a live sf::RenderWindow and graphical environment. Add an integration test later if "
	        "needed.");
}
