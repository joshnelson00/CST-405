# Refactoring Summary

## Issues Fixed

### 1. Arrays Not Executing ✅
**Problem**: Arrays compiled but printed 0 instead of correct values
**Root Causes**:
- No stack frame initialization in main
- `tempToReg()` used static buffer, causing all register pointers to point to the same value
- ADD instruction generated `add $t2, $t2, $t2` instead of `add $t2, $t0, $t1`

**Fixes**:
- Added stack frame allocation/deallocation in main based on symbol table's `nextOffset`
- Implemented rotating buffer in `tempToReg()` to support multiple concurrent register conversions
- Fixed ADD MIPS generation to use correct source registers

### 2. Multiple Function Calls Not Working ✅
**Problem**: Only the first function call was processed
**Root Cause**: MIPS generator searched for first FUNC_CALL and stopped

**Fix**:
- Refactored to iterate through entire main body
- Track pending ARG instructions before each call
- Generate MIPS for ALL function calls sequentially

### 3. Nested Function Calls Not Working ✅
**Problem**: Functions calling other functions failed
**Root Causes**:
- No `$ra` save/restore in functions that call others
- No tracking of variable-to-register mappings
- Single-argument function calls not handled
- Variables passed as arguments not loaded correctly

**Fixes**:
- Detect if function calls others, save/restore `$ra` on stack
- Implemented variable-to-temp mapping system within functions
- Handle 1-argument and 2+ argument cases
- Map variables to their source temp registers for correct loading
- Fixed RETURN to use variable mappings when returning variables

### 4. Array Parameters Not Working ✅
**Problem**: Array parameters existed in grammar but no MIPS generation
**Root Causes**:
- Array arguments passed values instead of addresses
- Array parameter accesses used wrong field mapping
- TAC structure fields misunderstood (arg1/arg2/result order)

**Fixes**:
- Pass array base address using `addi $a0, $sp, offset` 
- Added `isArray()` check to determine if passing address or value
- Implemented `TAC_ARRAY_READ` handling in function bodies
- Fixed field mapping: `arg1=arrayName, arg2=index, result=temp`
- Use `$a0` as base address for array parameter accesses

## Test Results

All tests now pass:

```bash
=== Basic Function ===
15                          # adding(12, 3) = 15 ✅

=== Arrays ===
30                          # arr[0]=10, arr[1]=20, arr[2]=10+20 ✅

=== Multiple Calls ===
12                          # addNum(5, 7) = 12 ✅
12                          # multiplyNum(3, 4) = 12 ✅

=== Nested Calls ===
30                          # compute(10) = (10+5)*2 = 30 ✅

=== Array Parameters ===
30                          # sumArray([10,20,30]) = 10+20 = 30 ✅
```

## Key Technical Improvements

1. **Stack Frame Management**: Proper allocation/deallocation based on symbol table
2. **Register Allocation**: Rotating buffer prevents pointer aliasing bugs
3. **Function Call Stack**: Proper `$ra` management for nested calls
4. **Variable Tracking**: Maps variables to their temp register locations
5. **Array Addressing**: Distinguishes between value passing and address passing

## Files Modified

- `optimizer2.c`: Major refactoring of MIPS generation logic
  - Stack frame initialization
  - Function body processing
  - Array parameter handling
  - Variable-to-register mapping
  - Rotating register buffer

## Remaining Limitations

- Array parameters beyond first parameter position not fully tested
- Stack-based local variables in functions with calls not implemented
- Register spilling not implemented (limited temp registers)
- Only basic arithmetic operations supported in functions
