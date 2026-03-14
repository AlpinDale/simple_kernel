# Testing Guide

## Test-First Development

When adding new features to this kernel, always write tests first:

1. **Define the behavior** - What should the feature do?
2. **Write the test** - Implement test cases for expected behavior
3. **Run and fail** - Verify tests fail initially (red)
4. **Implement feature** - Write minimal code to pass tests
5. **Run and pass** - Verify tests pass (green)
6. **Refactor** - Clean up code while keeping tests green

## Test Categories

### Unit Tests

Test individual functions and data structures in isolation.

**When to write:**
- Pure functions with clear inputs/outputs
- Data structure manipulation
- Bit manipulation and encoding/decoding logic

**Example:**
```c
TEST(vga_entry_creates_correct_value) {
  u16 entry = vga_entry('A', 0x0F);
  ASSERT_EQ(entry, 0x0F41);
  TEST_PASS_MSG();
}
```

### Integration Tests

Test system-level behavior using QEMU and the kernel serial console.

**When to write:**
- Boot sequence changes
- Hardware interaction (keyboard, screen)
- Mode transitions (real mode → protected → long mode)
- Interrupt handling

**Example:**
```bash
tests/integration/test_boot.sh
tests/integration/test_shell_io.sh
tests/integration/test_panic.sh
```

## Writing New Tests

### Unit Test Template

```c
// tests/unit/test_myfeature.c
#include "../test.h"
#include <stdint.h>

// Include necessary typedefs
typedef uint8_t u8;

// Your implementation under test
static u8 my_function(u8 input) {
    return input * 2;
}

TEST(my_function_doubles_input) {
    u8 result = my_function(5);
    ASSERT_EQ(result, 10);
    TEST_PASS_MSG();
}

TEST(my_function_handles_zero) {
    u8 result = my_function(0);
    ASSERT_EQ(result, 0);
    TEST_PASS_MSG();
}

int main(void) {
    printf("\nMy Feature Tests:\n");
    RUN_TEST(my_function_doubles_input);
    RUN_TEST(my_function_handles_zero);
    TEST_SUMMARY();
}
```

### Integration Test Template

```bash
tests/integration/test_boot.sh
```

## Available Assertions

```c
ASSERT(expr)              // Assert expression is true
ASSERT_EQ(a, b)          // Assert a == b
ASSERT_NEQ(a, b)         // Assert a != b
ASSERT_TRUE(expr)        // Assert expr is true
ASSERT_FALSE(expr)       // Assert expr is false
ASSERT_NULL(ptr)         // Assert ptr is NULL
ASSERT_NOT_NULL(ptr)     // Assert ptr is not NULL
```

## Running Tests

```bash
# All tests
make test

# Only unit tests (fast)
make test-unit

# Only integration tests (slower, requires QEMU)
make test-integration
```

## Best Practices

1. **Keep tests small** - One concept per test
2. **Test edge cases** - Zero, max values, boundaries
3. **Descriptive names** - `TEST(function_name_condition_expected_result)`
4. **Independent tests** - No shared state between tests
5. **Fast feedback** - Unit tests should be instant
6. **Document failures** - Clear error messages in assertions

## Test Coverage Goals

- [x] Boot sequence
- [x] VGA text mode
- [x] IDT setup and gates
- [x] Keyboard input
- [x] Shell serial I/O
- [x] Panic path
- [x] Physical memory manager basics
