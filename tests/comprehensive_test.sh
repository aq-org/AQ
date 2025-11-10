#!/bin/bash

# Comprehensive test suite for the refactored interpreter

echo "=========================================="
echo "COMPREHENSIVE TEST SUITE"
echo "=========================================="
echo ""

PASS_COUNT=0
FAIL_COUNT=0

# Test 1: Simple program without imports
echo "Test 1: Simple program without imports"
cat > /tmp/test1.aq << 'EOAQ'
auto main(){
    __builtin_print("Hello World");
}
EOAQ
if ./build/aq /tmp/test1.aq 2>&1 | grep -q "Hello World"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 2: Global variables
echo "Test 2: Global variables"
cat > /tmp/test2.aq << 'EOAQ'
auto global_var = "GLOBAL";
auto main(){
    __builtin_print(global_var);
}
EOAQ
if ./build/aq /tmp/test2.aq 2>&1 | grep -q "GLOBAL"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 3: Global functions
echo "Test 3: Global functions"
cat > /tmp/test3.aq << 'EOAQ'
auto test_func(){
    __builtin_print("FUNC_OK");
}
auto main(){
    test_func();
}
EOAQ
if ./build/aq /tmp/test3.aq 2>&1 | grep -q "FUNC_OK"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 4: Lambda functions
echo "Test 4: Lambda functions"
cat > /tmp/test4.aq << 'EOAQ'
auto lambda_var = func(){
    __builtin_print("LAMBDA_OK");
}
auto main(){
    lambda_var();
}
EOAQ
if ./build/aq /tmp/test4.aq 2>&1 | grep -q "LAMBDA_OK"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 5: Simple import - variable access
echo "Test 5: Simple import - variable access"
cat > /tmp/test5_imported.aq << 'EOAQ'
auto main(){}
auto exported_var = "IMPORT_VAR_OK";
EOAQ
cat > /tmp/test5.aq << 'EOAQ'
import "/tmp/test5_imported.aq" imported;
auto main(){
    __builtin_print(imported.exported_var);
}
EOAQ
if ./build/aq /tmp/test5.aq 2>&1 | grep -q "IMPORT_VAR_OK"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 6: Simple import - variable modification
echo "Test 6: Simple import - variable modification"
cat > /tmp/test6_imported.aq << 'EOAQ'
auto main(){}
auto exported_var = "BEFORE";
EOAQ
cat > /tmp/test6.aq << 'EOAQ'
import "/tmp/test6_imported.aq" imported;
auto main(){
    imported.exported_var = "AFTER";
    __builtin_print(imported.exported_var);
}
EOAQ
if ./build/aq /tmp/test6.aq 2>&1 | grep -q "AFTER"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 7: Simple import - function call
echo "Test 7: Simple import - function call"
cat > /tmp/test7_imported.aq << 'EOAQ'
auto main(){}
auto exported_func(){
    __builtin_print("IMPORT_FUNC_OK");
}
EOAQ
cat > /tmp/test7.aq << 'EOAQ'
import "/tmp/test7_imported.aq" imported;
auto main(){
    imported.exported_func();
}
EOAQ
if ./build/aq /tmp/test7.aq 2>&1 | grep -q "IMPORT_FUNC_OK"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 8: Simple import - lambda call
echo "Test 8: Simple import - lambda call"
cat > /tmp/test8_imported.aq << 'EOAQ'
auto main(){}
auto exported_lambda = func(){
    __builtin_print("IMPORT_LAMBDA_OK");
}
EOAQ
cat > /tmp/test8.aq << 'EOAQ'
import "/tmp/test8_imported.aq" imported;
auto main(){
    imported.exported_lambda();
}
EOAQ
if ./build/aq /tmp/test8.aq 2>&1 | grep -q "IMPORT_LAMBDA_OK"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 9: Nested imports (issue1)
echo "Test 9: Nested imports (issue1)"
if ./build/aq tests/issue1/main.aq 2>&1 | grep -q "F1F1F1"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

# Test 10: issue2 - partial (lines 4-7)
echo "Test 10: issue2 - variable and lambda tests"
cat > /tmp/test10.aq << 'EOAQ'
import "tests/issue2/test.aq" test;
auto main(){
    __builtin_print(test.test_global_var);
    test.test_global_var = 100;
    __builtin_print(test.test_global_var);
    test.test_function();
}
EOAQ
OUTPUT=$(./build/aq /tmp/test10.aq 2>&1)
if echo "$OUTPUT" | grep -q "SUCCESS" && echo "$OUTPUT" | grep -q "100" && echo "$OUTPUT" | grep -q "lambda test OK"; then
    echo "✓ PASS"
    ((PASS_COUNT++))
else
    echo "✗ FAIL"
    ((FAIL_COUNT++))
fi
echo ""

echo "=========================================="
echo "SUMMARY: $PASS_COUNT passed, $FAIL_COUNT failed"
echo "=========================================="
