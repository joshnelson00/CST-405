# MiniC Compiler

A teaching compiler for a C-like language that translates source programs into MIPS assembly and optionally runs them through SPIM for transcript and execution-time reporting.

## What You Get

- Full compiler pipeline: scanner -> parser -> AST -> TAC -> optimizer -> MIPS codegen
- Rich phase-by-phase diagnostics in terminal output
- Generated artifacts for both unoptimized and optimized flows
- Compilation-time and execution-time measurements in report output

## Project Layout

- `main.c`: compiler driver, phase orchestration, benchmark/report generation
- `scanner.l`: lexical analyzer (Flex)
- `parser.y`: parser and grammar (Bison)
- `ast.*`: AST structures and helpers
- `symtab.*`: symbol table and semantic tracking
- `tac.*`: three-address code generation and dumping
- `optimizer.*`: TAC optimization passes
- `mips.*`: MIPS emission helpers
- `benchmark.*`: wall/CPU/memory benchmarking helpers
- `Makefile`: build, clean, and test targets

## Prerequisites

You need the following tools installed:

- C compiler (`gcc` or `clang`)
- `make`
- `flex`
- `bison`
- SPIM simulator (`spim`) for executing generated MIPS assembly

If SPIM is not installed, compilation still works, but execution transcript/timing steps will fail.

## Installation (Linux, macOS, Windows)

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential flex bison spim make
```

### Linux (Fedora/RHEL)

```bash
sudo dnf install -y gcc gcc-c++ make flex bison spim
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

Notes for macOS:

- Apple ships older `flex`/`bison` versions. Homebrew versions are recommended.
- If needed, add them to your shell path:

```bash
export PATH="$(brew --prefix bison)/bin:$(brew --prefix flex)/bin:$PATH"
```

### Windows Option A (Recommended): WSL2 Ubuntu

1. Install WSL2 and Ubuntu.
2. Open Ubuntu terminal and run:

```bash
sudo apt update
sudo apt install -y build-essential flex bison spim make
```

Then build/run exactly like Linux in WSL.

### Windows Option B: MSYS2/MinGW (advanced)

1. Install MSYS2.
2. Open `MSYS2 UCRT64` shell.
3. Install packages:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc make flex bison
```

SPIM availability on native Windows can vary; WSL is usually simpler and more consistent for this project.

## Build Instructions

From the repository root:

### Linux / macOS / WSL

```bash
make clean
make
```

### Windows PowerShell (in WSL folder or Git Bash/MSYS shell)

```powershell
make clean
make
```

Expected result: `minicompiler` binary is created in the project root.

## Usage Instructions

Compiler usage:

```bash
./minicompiler <input.c> <output.s>
```

Example:

```bash
./minicompiler test_final.c test_final.s
```

### Linux / macOS / WSL Run Flow

1. Compile source to MIPS:

```bash
./minicompiler test_final.c test_final.s
```

2. Optionally run MIPS manually in SPIM:

```bash
spim -file test_final.s
```

### Windows Run Flow

#### WSL (recommended)

```bash
./minicompiler test_final.c test_final.s
spim -file test_final.s
```

#### MSYS2/Git Bash style

```bash
./minicompiler test_final.c test_final.s
spim -file test_final.s
```

(Use the shell where your toolchain was installed.)

## Output Files

After a successful run, expect artifacts such as:

- `<output>.s`: generated optimized MIPS assembly
- `test_final_unopt.s`: unoptimized reference MIPS output (when generated)
- `tac_unopt.txt`: unoptimized TAC listing
- `tac_opt.txt`: optimized TAC listing
- `output_transcript.txt`: SPIM transcript (optimized)
- `output_transcript_unopt.txt`: SPIM transcript (unoptimized)
- `report.txt`: phase timings, compile/execution metrics, optimization stats

## Benchmark and Timing Notes

The compiler reports:

- Phase-by-phase CPU and wall time
- Compilation-time summary
- Execution-time summary (SPIM)
- End-to-end totals in `report.txt`

Execution timing is comparable only when both SPIM runs (unoptimized and optimized) succeed.

## Common Commands

Build:

```bash
make
```

Clean:

```bash
make clean
```

Run bundled tests:

```bash
make test-all
```

## Troubleshooting

- `spim: command not found`
  - Install SPIM for your platform, then re-run.
- `flex` or `bison` not found
  - Install required packages and ensure they are on your PATH.
- macOS uses older system `bison`/`flex`
  - Install Homebrew versions and prepend Homebrew paths.
- Windows native tool mismatch
  - Prefer WSL2 Ubuntu for consistent behavior.

## Features

- Lexing and parsing for a C-like educational language
- AST construction and semantic checks
- Three-address code (TAC) generation
- Optimization passes including constant folding/copy propagation/dead code removal
- MIPS assembly generation from optimized TAC
- Unoptimized vs optimized artifact generation for comparison
- SPIM transcript generation for runtime verification
- Detailed performance reporting for compile and execution phases
