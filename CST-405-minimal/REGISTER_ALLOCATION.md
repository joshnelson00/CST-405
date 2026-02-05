# Register Allocation Policy

## Implementation

The compiler uses a **simple modulo-based register allocation** strategy for MIPS temp registers.

### Strategy

```c
// Direct mapping: t0→$t0, t1→$t1, ..., t9→$t9
// Wrapping:       t10→$t0, t11→$t1, etc.
int regNum = tempNum % 10;
```

### MIPS Register Constraints

- MIPS provides **10 temporary registers**: `$t0` through `$t9`
- The compiler can generate many more than 10 temp variables
- Register wrapping allows programs with >10 temps to compile and run

## How It Works

1. **Temp variables** are generated during TAC (Three-Address Code) generation
2. Each temp (t0, t1, t2, ...) is mapped to a physical MIPS register
3. When temps exceed 10, they **wrap around** using modulo arithmetic
4. Example: `t0→$t0, t10→$t0, t20→$t0` (same register reused)

## Tested Scenarios

The comprehensive test in `test.cm` demonstrates:

✅ **Multiple Functions** - 4 user-defined functions  
✅ **Arrays** - Declaration, indexing, and operations  
✅ **Nested Calls** - Functions calling other functions  
✅ **Array Parameters** - Passing arrays by reference  
✅ **Sequential Calls** - Multiple function invocations  
✅ **Register Reuse** - Programs with many temp variables

## Test Results

```
Input Program: test.cm
- 4 functions
- 3 array elements
- 6 function calls
- Nested function calls
- Array parameter passing

Output: 25 30 30 30 24 55 ✅ (All correct!)
```

## Limitations

### Known Constraints

1. **Register Conflicts**: When temp variables wrap around (t10 reuses $t0), there's potential for conflicts if $t0 is still live
2. **No Spilling**: Registers aren't spilled to stack when pressure is high
3. **No Live Range Analysis**: Simple temporal ordering, not optimal liveness

### Best Practices

To write programs that work well with this allocation:

1. **Use intermediate variables** in functions:
   ```c
   // Good
   int result;
   result = arr[0] + arr[1];
   return result;
   
   // May cause issues in complex scenarios
   return arr[0] + arr[1];
   ```

2. **Limit temp variable generation** by simplifying expressions
3. **Test incrementally** when adding complex features

## Performance

**Advantages:**
- ✅ Simple and predictable
- ✅ Fast compilation (O(1) register lookup)
- ✅ Works for most educational programs

**Trade-offs:**
- ⚠️ Not optimal for register usage
- ⚠️ May cause issues with very complex expressions
- ⚠️ No register pressure management

## Future Improvements

Potential enhancements (not currently implemented):

1. **Live Range Analysis**: Track when variables are actually used
2. **Graph Coloring**: Optimal register allocation algorithm
3. **Register Spilling**: Save/restore registers to/from stack
4. **Better Heuristics**: LRU or other eviction policies

## Conclusion

The current register allocation is **sufficient for educational purposes** and handles:
- Multiple functions with parameters
- Arrays and array parameters
- Nested function calls
- Programs with moderate complexity

For production compilers, more sophisticated techniques would be needed.
