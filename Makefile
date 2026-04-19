.PHONY: all debug release test run clean run_release test_release run_clang_tidy run_clang_tidy_fix

all: debug

# Debug
debug:
	cmake --preset debug
	cmake --build --preset debug

run: debug
	./build/debug/app

test: debug
	ctest --preset debug --output-on-failure

# Release
release:
	cmake --preset release
	cmake --build --preset release

run_release: release
	./build/release/app

test_release: release
	ctest --preset release --output-on-failure

run_clang_tidy: debug
	run-clang-tidy -p build/debug

run_clang_tidy_fix: debug
	run-clang-tidy -p build/debug -fix

clean:
	rm -rf build