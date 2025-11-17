# CoAP Client Scaffold

This repository is intentionally minimal so you can implement the CoAP Lab 1 client completely from scratch. Only the tooling glue (CMake project, a placeholder `main`, and a simple test harness) is provided so you can focus on your own protocol implementation.

## Current Layout

- `src/main.cpp` – prints a placeholder message; replace with your real client logic.
- `tests/assert.hpp` – tiny `EXPECT` macro to keep dependencies out.
- `tests/smoke_tests.cpp` – verifies the custom assertion helper works; feel free to add your own tests.
- `CMakeLists.txt` – builds the `coap_client` executable plus the `unit_tests` target with `Debug` as the default configuration.

## Build & Test

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build
```

The last command runs the smoke test to ensure the toolchain is wired up. Expand it with your own tests as you implement the CoAP features.

## Next Steps

1. Design your message/transport abstractions (headers under `include/` or wherever you prefer).
2. Flesh out `src/main.cpp` to parse CLI arguments and drive your CoAP logic.
3. Add unit/integration tests to `tests/` and register them in `CMakeLists.txt`.

Use this scaffold as a clean slate and iterate however you like.
