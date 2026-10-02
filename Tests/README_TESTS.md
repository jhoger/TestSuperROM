# THOUGHT Test Framework

This directory contains the Unity-based unit tests for the THOUGHT outline processor.

## Project Structure

```
Tests/
├── test_thought_driver.c      # Main test driver entry point
├── test_thought_outline.c/h    # Outline functionality tests
├── test_thought_navigation.c/h # Navigation tests
├── test_thought_modes.c/h      # Mode transition tests
├── test_thought_file_io.c/h    # File I/O tests
└── test_thought_print.c/h      # Print functionality tests

Common/
├── thought_test_common.c/h     # Common test utilities and fixtures

Tools/
├── unity/                      # Unity framework (to be downloaded)
└── README_TESTS.md             # This file
```

## Setup

1. **Download Unity**: The Unity test framework is required. Download it from:
   ```
   https://github.com/ThrowTheSwitch/Unity/releases
   ```
   Place the `unity` directory in `Tools/unity/`

2. **Build with CMake**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Run tests**:
   ```bash
   ./thought_tests
   ```

## Test Categories

Tests are organized into the following categories:

1. **Outline Tests** - Create, delete, expand, collapse outline entries
2. **Navigation Tests** - Cursor movement and view management
3. **Mode Tests** - Transitions between Create, Review/Revise, and Document modes
4. **File I/O Tests** - Save/load outline and document files
5. **Print Tests** - Print format options and defaults

## Adding New Tests

1. Create `test_<module>.c` and `test_<module>.h` in the Tests directory
2. Add test functions following the naming convention: `test_<functionality>`
3. Include the header in `test_thought_driver.c`
4. Register the test in `main()`

## Test Patterns

### Basic Test Pattern
```c
void test_<functionality>(void) {
    /* Setup */
    /* Act */
    /* Assert */
}
```

### Fixture Pattern
```c
void test_<functionality>(void) {
    thought_fixture_t fixture;
    thought_fixture_setup(&fixture);
    
    /* Test code */
    
    thought_fixture_teardown(&fixture);
}
```

## Continuous Integration

Tests can be run via CMake:
```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```