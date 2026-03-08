#include <catch2/catch_test_macros.hpp>

#include "mylib/foo.hpp"

TEST_CASE("Foo Test", "[foo]")
{
	REQUIRE(mylib::foo() == 42);
}
