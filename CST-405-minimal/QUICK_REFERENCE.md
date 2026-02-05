# Quick Reference Guide

## ✅ WHAT WORKS NOW

### Compile & Run
```bash
# Build compiler
make clean && make

# Compile a program
./minicompiler input.cm output.s

# Run with SPIM
spim -file output.s
```

### Working Example
```c
// test.cm - WORKS PERFECTLY
int adding(int x, int y) {
    return x + y;
}

void main() {
    int sum;
    sum = adding(12, 3);
    print(sum);  // Outputs: 15
}
```

### Supported Features
- ✅ Function declarations (int, float, void)
- ✅ Function parameters (int, float, int[], float[])
- ✅ Function calls with arguments
- ✅ Return statements
- ✅ Variable declarations
- ✅ Arithmetic operations (+, -, *, /)
- ✅ Print statements
- ✅ Array declarations: `int arr[10];`
- ✅ Array access in expressions (TAC level)
- ✅ Array assignment (TAC level)

## ⚠️ LIMITATIONS

### Arrays
- **Grammar & TAC**: ✅ Complete
- **MIPS Execution**: ❌ Not working (stack frame issues)
- **Workaround**: Arrays generate correct TAC but don't execute

### Function Calls
- **Single Call**: ✅ Works
- **Multiple Calls**: ❌ Only first call processed
- **Nested Calls**: ❌ Not supported

### Register Allocation
- **Basic**: ✅ Simple temp→register mapping
- **Spilling**: ❌ Not implemented
- **Optimization**: ❌ Not implemented

## 📝 GRAMMAR REFERENCE

### Variable Declaration
```c
int x;
float y;
int arr[10];
float values[5];
```

### Function Definition
```c
int func(int a, float b) {
    // body
    return value;
}

void proc(int arr[], int size) {
    // array parameter supported in grammar
}
```

### Expressions
```c
x = a + b;
result = x * y - z;
arr[0] = 10;        // TAC works, MIPS doesn't
value = arr[i];     // TAC works, MIPS doesn't
```

### Function Call
```c
result = func(10, 3.14);
print(result);
```

## 🐛 KNOWN ISSUES

1. **Arrays in MIPS**: Stack not initialized, causes bad addresses
2. **Nested Calls**: Only first function call works
3. **Multiple Prints**: May print twice (extra newline)
4. **Array Parameters**: Grammar done, code gen not implemented

## 🔧 FILES TO CHECK

- `parser.y` - Grammar rules
- `test.cm` - Working test case
- `test_arrays.cm` - Array test (TAC only)
- `FINAL_STATUS.md` - Complete status
- `IMPLEMENTATION_STATUS.md` - Detailed breakdown

## 💻 DEVELOPMENT TIPS

### Adding New Features
1. Update `parser.y` grammar
2. Add AST nodes in `ast.h/c`
3. Generate TAC in `tac.c`
4. Implement MIPS in `optimizer2.c`
5. Test incrementally

### Debugging
```bash
# Check TAC output
./minicompiler test.cm mips.s 2>&1 | grep -A 20 "Unoptimized TAC"

# Check MIPS output
cat mips.s

# Run with SPIM
spim -file mips.s
```

### Common Errors
- **"Bad data address"**: Stack frame not initialized
- **"Syntax error"**: MIPS generation bug
- **Wrong output**: Check TAC first, then MIPS

## 📊 WHAT TO USE THIS FOR

### ✅ Good For:
- Learning compiler phases
- Understanding TAC generation
- Testing grammar changes
- Simple programs with one function call
- Demonstrating parser design

### ❌ Not Ready For:
- Complex programs
- Production use
- Programs with multiple function calls
- Array manipulation in MIPS
- Register pressure scenarios

---

**Quick Test**: 
```bash
./minicompiler test.cm mips.s && spim -file mips.s
# Should output: 15
```
