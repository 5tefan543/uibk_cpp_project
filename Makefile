.PHONY: all debug release test run clean run_release test_release

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

clean:
	rm -rf build