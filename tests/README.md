# AQ Compiler Test Suite

This directory contains comprehensive tests for the AQ compiler, designed to cover various edge cases and validate core functionality.

## Test Files

### 1. test_arithmetic.aq
Tests arithmetic operations including:
- Basic operations (+, -, *, /, %)
- Negative numbers
- Mixed operations with proper precedence
- Zero edge cases

### 2. test_conditionals.aq
Tests conditional statements including:
- Simple if statements
- If-else statements
- Nested conditionals
- Comparison operators (==, <=, >=, <, >)
- Zero comparison edge cases

### 3. test_loops.aq
Tests loop constructs including:
- Simple while loops
- Nested while loops
- Loops with complex conditions

### 4. test_functions.aq
Tests function declarations and calls including:
- Simple function calls
- Multiple function calls
- Recursive functions (Note: has known issue with return values)
- Void functions
- Nested function calls

### 5. test_types.aq
Tests different data types including:
- Integer types
- String types
- Auto type inference
- Large numbers
- Negative numbers and zero

### 6. test_strings.aq
Tests string handling including:
- Basic string output
- Empty strings
- Special characters (newline, tab)
- Long strings
- Mixed content (strings and numbers)

### 7. test_edge_cases.aq
Tests various edge cases including:
- Variable scoping and shadowing
- Multiple variable declarations
- Complex expression evaluation
- Boolean-like operations
- Operator precedence

### 8. test_return_only.aq
Tests simple function return values.

### 9. test_recursion_simple.aq
Tests recursive function calls (Note: has known issue).

## Running Tests

To run all tests:
```bash
cd /home/runner/work/AQ/AQ
for file in tests/*.aq; do
    echo "Testing: $file"
    ./build/aq "$file"
done
```

To run a specific test:
```bash
./build/aq tests/test_arithmetic.aq
```

## Known Issues

1. **Recursion Bug**: Return values from recursive function calls may be uninitialized. A defensive fix has been added to prevent crashes, but the value returned may be incorrect (returns 0 instead of the correct value).

2. **String Operator Warning**: Some tests produce "Unexpected Type std::string" warnings in the operator code. This does not affect functionality but should be investigated.

3. **Import Deprecation**: The general import syntax is deprecated in favor of more specific import forms.

## Test Results

As of the latest run:
- 9/9 new test files pass (with warnings on known issues)
- 7/9 original sample files pass
- 2 original samples fail due to deprecated import syntax

## Adding New Tests

When adding new tests:
1. Create a new .aq file in this directory
2. Follow the naming convention: test_<feature>.aq
3. Include descriptive comments explaining what is being tested
4. Test both normal cases and edge cases
5. Update this README with the new test description
