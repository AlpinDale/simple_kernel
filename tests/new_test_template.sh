#!/bin/bash
# Quick test scaffolding script

if [ $# -ne 2 ]; then
    echo "Usage: $0 <test-type> <test-name>"
    echo "  test-type: unit or integration"
    echo "  test-name: name of the test (e.g., memory, paging)"
    exit 1
fi

TYPE=$1
NAME=$2

if [ "$TYPE" = "unit" ]; then
    cat > "tests/unit/test_${NAME}.c" << 'TEMPLATE'
#include "../test.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Your implementation or mock here

TEST(example_test) {
    ASSERT_EQ(1, 1);
    TEST_PASS_MSG();
}

int main(void) {
    printf("\n${NAME^} Tests:\n");
    RUN_TEST(example_test);
    TEST_SUMMARY();
}
TEMPLATE
    echo "Created tests/unit/test_${NAME}.c"
    echo "Add to Makefile test-unit target:"
    echo "  @gcc tests/unit/test_${NAME}.c -o build/test_${NAME} && ./build/test_${NAME}"

elif [ "$TYPE" = "integration" ]; then
    cat > "tests/integration/test_${NAME}.sh" << 'TEMPLATE'
#!/bin/bash
set -e

echo "Integration Test: ${NAME^}"
echo "================================"

# Your QEMU test here

echo "✓ Test passed"
TEMPLATE
    chmod +x "tests/integration/test_${NAME}.sh"
    echo "Created tests/integration/test_${NAME}.sh"
    echo "Add to Makefile test-integration target:"
    echo "  @./tests/integration/test_${NAME}.sh"
else
    echo "Invalid test type: $TYPE"
    exit 1
fi
