#!/bin/bash
# AQ Compiler Test Runner
# Runs all test files in the tests directory and reports results

echo "================================"
echo "AQ Compiler Test Suite"
echo "================================"
echo ""

TESTS_DIR="$(dirname "$0")"
AQ_BINARY="${TESTS_DIR}/../build/aq"

if [ ! -f "$AQ_BINARY" ]; then
    echo "Error: AQ compiler not found at $AQ_BINARY"
    echo "Please build the compiler first: cd .. && mkdir -p build && cd build && cmake .. && make"
    exit 1
fi

PASSED=0
FAILED=0
TOTAL=0

for test_file in "$TESTS_DIR"/*.aq; do
    [ -f "$test_file" ] || continue
    
    TOTAL=$((TOTAL + 1))
    test_name=$(basename "$test_file")
    
    echo -n "Testing $test_name ... "
    
    if "$AQ_BINARY" "$test_file" > /tmp/aq_test_output.txt 2>&1; then
        echo "✓ PASS"
        PASSED=$((PASSED + 1))
    else
        echo "✗ FAIL"
        FAILED=$((FAILED + 1))
        echo "  Output:"
        cat /tmp/aq_test_output.txt | head -10
    fi
done

echo ""
echo "================================"
echo "Test Results"
echo "================================"
echo "Total:  $TOTAL"
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo ""

if [ $FAILED -eq 0 ]; then
    echo "All tests passed! ✓"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
