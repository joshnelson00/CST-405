# CST-405 Minimal C Compiler

A teaching compiler for a C-like language that compiles source programs into MIPS assembly.

Pipeline overview:

Source (.cm) -> Scanner (Flex) -> Parser (Bison) -> AST -> TAC -> Optimizer -> MIPS (.s)

## What This Compiler Includes

- Lexical analysis using `scanner.l` (Flex)
- Parsing and semantic checks using `parser.y` (Bison)
- AST construction and traversal
- TAC generation and serialization
- TAC optimization pass with branch and loop-aware improvements
- MIPS code generation
- Phase-by-phase benchmark/report output
- Optional SPIM transcript capture (`output_transcript.txt`)

## Key Project Files

- `main.c`: Driver for all compiler phases and report generation
- `scanner.l`: Lexer specification
- `parser.y`: Grammar + semantic rule actions
- `ast.c` / `ast.h`: AST data model and utilities
- `symtab.c` / `symtab.h`: Symbol table implementation
- `tac.c` / `tac.h`: Three-address code generation
- `optimizer.c` / `optimizer.h`: TAC optimization + optimized MIPS path
- `mips.c` / `mips.h`: MIPS helpers
- `benchmark.c` / `benchmark.h`: Timing/memory benchmark helpers
- `Makefile`: Build, clean, and test targets

## Requirements

Install these tools first:

- GCC (or compatible C compiler)
- Make
- Flex
- Bison
- SPIM (optional but recommended for running generated MIPS)

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential make flex bison spim
```

### Linux (Fedora)

```bash
sudo dnf install -y gcc make flex bison spim
```

### Linux (Arch)

```bash
sudo pacman -S --needed base-devel flex bison spim
```

### macOS (Homebrew)

```bash
brew update
brew install flex bison spim
```

If needed on macOS, ensure Homebrew tool paths are available in your shell:

```bash
echo 'export PATH="/opt/homebrew/opt/flex/bin:/opt/homebrew/opt/bison/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Windows Option 1 (WSL - Recommended)

Install WSL with Ubuntu, open the Ubuntu terminal, then run:

```bash
sudo apt update
sudo apt install -y build-essential make flex bison spim
```

### Windows Option 2 (MSYS2)

In the MSYS2 UCRT64 terminal:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc make flex bison
```

Note: SPIM may be easiest to use via WSL or a Linux VM if unavailable in your native Windows toolchain.

Verify tools:

```bash
gcc --version
make --version
flex --version
bison --version
spim -v
```

## Installation and Build

1. Clone or open this project in your workspace.
2. From the project root, build the compiler:

```bash
make
```

This creates the executable:

- `./minicompiler`

## Basic Usage

Compile one source file into MIPS:

```bash
./minicompiler <input.cm> <output.s>
```

Example:

```bash
./minicompiler test_final.c test_final.s
```

During compilation, the compiler prints each phase and writes artifacts to disk.

## Output Artifacts

After a successful compile, you should see:

- `tac_unopt.txt`: Unoptimized TAC dump
- `tac_opt.txt`: Optimized TAC dump
- `<output>.s`: Generated MIPS assembly (for example, `test_final.s`)
- `report.txt`: Compilation summary with per-phase timing/memory
- `output_transcript.txt`: SPIM output transcript (if SPIM executes successfully)

Important behavior for auditing:

- MIPS code generation always uses optimized TAC.
- Unoptimized TAC is emitted for inspection only and is not used for code generation.
- This is explicitly documented in both runtime console output and `report.txt`.

## Run Generated Assembly

Execute output in SPIM:

```bash
spim -file test_final.s
```

Or inspect captured transcript from compiler run:

```bash
cat output_transcript.txt
```

## Build and Utility Targets

Available Make targets include:

- Build compiler:

```bash
make
```

- Clean generated files:

```bash
make clean
```

## Typical Development Workflow

1. Build:

```bash
make
```

2. Compile sample program:

```bash
./minicompiler test_final.c test_final.s
```

3. Review intermediate forms:

```bash
cat tac_unopt.txt
cat tac_opt.txt
```

4. Run output:

```bash
spim -file test_final.s
```

5. Check report:

```bash
cat report.txt
```

## Troubleshooting

Build errors:

- If `flex` or `bison` is missing, install dependencies and rebuild.
- If parser/lexer artifacts are stale, run `make clean` then `make`.

Runtime errors:

- If SPIM is not installed, compilation still works, but transcript capture/run steps may fail.
- If input path is wrong, compiler will report file open failure.

Validation issues:

- Read terminal diagnostics first (syntax/semantic errors include guidance).
- Then review `report.txt` for phase status and totals.

## Notes

- This compiler is educational and intentionally verbose in phase output.
- Performance benchmark lines are included per phase and in total.
- The source language and test coverage are designed for class project use, not production C compatibility.
