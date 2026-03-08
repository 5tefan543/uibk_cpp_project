## Prerequisites

### Pre-commit checks
To ensure code quality and consistency, we use the clang-format pre-commit hook. Install the pre-commit package and set up the hook:
```bash
pipx install pre-commit
pre-commit install
```

### CMake & VCPKG
Make sure you have [CMake](https://cmake.org/download/) installed on your system. Additionally, we use [VCPKG](https://github.com/Microsoft/vcpkg) for dependency management. In order to install and use VCPKG with cmake and Visual Studio Code, follow this [guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started-vscode?pivots=shell-bash).


## Build Instructions
1. Configure the project using CMake presets:
```bash
cmake --preset debug
cmake --preset release
```
2. Build the project using the configured presets:
```bash
cmake --build --preset debug
cmake --build --preset release
```
3. Run the application in the respective build directories:
```bash
./build/debug/myapp
./build/release/myapp
```

## Running Tests
To run the tests, use the following command:
```bash
ctest --preset debug
ctest --preset release
```