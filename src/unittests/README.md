# Unit Tests for unix.lib

This directory contains unit tests for the unix.lib functions, specifically testing the 64-bit arithmetic and string conversion functions.

## Test Programs

### test_longlong
Comprehensive test suite for the entire longlong library that verifies:
- **Basic Functions**: `long_to_long_long`, `get_long_long_max`, `get_long_long_min`
- **Arithmetic Operations**: Addition, subtraction, multiplication, division
- **Comparison Functions**: Equality, less-than, greater-than operations
- **String Conversion**: `strtoll` function with all edge cases
- **Critical Cases**: LONG_LONG_MIN parsing, overflow detection, invalid inputs
- **Boundary Conditions**: Carry/borrow operations, 32-bit boundary crossing

### test_strtoll
Comprehensive test suite for the `strtoll()` function that verifies:
- Basic decimal parsing
- Large 32-bit numbers
- Numbers beyond 32-bit range
- Maximum 64-bit positive number
- Negative numbers with proper two's complement
- Hexadecimal and other base parsing
- Auto-base detection
- Overflow detection and error handling
- **Critical LONG_LONG_MIN case** (-9223372036854775808)
- Invalid base handling

### test_overflow
Test program for the overflow detection logic in `strtoll()`, verifying:
- Safe multiplication scenarios
- Boundary conditions
- Overflow detection accuracy
- Edge cases with different bases and digits

## Building Tests

### From the main src directory (using smake)
```bash
smake unittests
```

## Running Tests

### Run all tests
```bash
make test
```

### Run tests with shell script
```bash
run_tests
```

### Run individual tests
```bash
smake test-strtoll
smake test-overflow
```

### Run tests manually
```bash
test_strtoll
test_overflow
```

## Test Output

### test_strtoll
The test program runs 11 test cases and reports:
- Individual test results
- Expected vs. actual values
- Error counts
- Overall success/failure status

Example output:
```
Test 1: Basic decimal parsing
Result: hi=0x00000000, lo=0x00003039
Expected: hi=0x00000000, lo=0x00003039

Test Summary: 0 errors found
Total tests run: 11
SUCCESS: All tests passed!
```

### test_overflow
The overflow test program shows:
- Test combinations of current values, bases, and digits
- Overflow detection logic results
- Safe vs. unsafe operation identification

## Dependencies

The test programs depend on:
- `/longlong.h` - Header file with function prototypes
- `/longlong.o` - Compiled 64-bit arithmetic functions

## Cleanup

To remove test files:
```bash
make clean
```

## Integration

These tests are automatically built when running `smake` from the parent directory, ensuring that the main library build includes comprehensive testing.

## Notes

- Tests are designed to run on AmigaOS with SAS/C
- All tests follow C89/ANSI C standards
- Test results are displayed in hexadecimal format for easy debugging
- The critical LONG_LONG_MIN test verifies the fix for the asymmetric range bug
