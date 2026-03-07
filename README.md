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