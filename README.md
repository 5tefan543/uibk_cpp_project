## Build Instructions
1. Configure the project with CMake:
   ```bash
   cmake -S . -B build
   ```
2. Build the project:
   ```bash
   cmake --build build --target mylib_myapp
   ```
3. Run the executable:
   ```bash
   ./build/myapp
   ```

## Prerequisites

### Pre-commit checks
To ensure code quality and consistency, we use the clang-format pre-commit hook. Install the pre-commit package and set up the hook:
```bash
pipx install pre-commit
pre-commit install
```