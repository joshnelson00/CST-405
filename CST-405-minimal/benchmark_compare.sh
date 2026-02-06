#!/bin/bash
# Compiler Performance Comparison Script
# Usage: ./benchmark_compare.sh test_file.cm

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <test_file.cm>"
    echo "Example: $0 test.cm"
    exit 1
fi

TEST_FILE=$1
BASELINE="baseline_benchmark.txt"
CURRENT="current_benchmark.txt"

echo "╔════════════════════════════════════════════════════════════╗"
echo "║         COMPILER PERFORMANCE COMPARISON                    ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Test File: $TEST_FILE"
echo ""

# Run compiler and extract performance metrics
./minicompiler "$TEST_FILE" test_output.s 2>&1 | grep -E "Performance|CPU Time|Wall Time|Memory" > "$CURRENT"

if [ ! -f "$BASELINE" ]; then
    echo "⚠️  No baseline found. Creating baseline..."
    cp "$CURRENT" "$BASELINE"
    echo "✅ Baseline created at: $BASELINE"
    echo ""
    echo "Run this script again after making optimizations to compare."
    exit 0
fi

echo "════════════════════════════════════════════════════════════"
echo "  BASELINE vs CURRENT"
echo "════════════════════════════════════════════════════════════"
echo ""

# Extract total times
BASELINE_CPU=$(grep "TOTAL" -A 1 "$BASELINE" | grep "CPU Time" | awk '{print $3}')
CURRENT_CPU=$(grep "TOTAL" -A 1 "$CURRENT" | grep "CPU Time" | awk '{print $3}')

BASELINE_WALL=$(grep "TOTAL" -A 2 "$BASELINE" | grep "Wall Time" | awk '{print $3}')
CURRENT_WALL=$(grep "TOTAL" -A 2 "$CURRENT" | grep "Wall Time" | awk '{print $3}')

BASELINE_MEM=$(grep "TOTAL" -A 3 "$BASELINE" | grep "Memory" | awk '{print $3}')
CURRENT_MEM=$(grep "TOTAL" -A 3 "$CURRENT" | grep "Memory" | awk '{print $3}')

echo "TOTAL COMPILATION TIME:"
echo "  Baseline CPU:  ${BASELINE_CPU}s"
echo "  Current CPU:   ${CURRENT_CPU}s"
echo ""
echo "  Baseline Wall: ${BASELINE_WALL}s"
echo "  Current Wall:  ${CURRENT_WALL}s"
echo ""
echo "  Baseline Mem:  ${BASELINE_MEM} KB"
echo "  Current Mem:   ${CURRENT_MEM} KB"
echo ""

# Calculate improvement (using bc for floating point)
if command -v bc > /dev/null 2>&1; then
    CPU_IMPROVEMENT=$(echo "scale=2; (($BASELINE_CPU - $CURRENT_CPU) / $BASELINE_CPU) * 100" | bc 2>/dev/null || echo "0")
    WALL_IMPROVEMENT=$(echo "scale=2; (($BASELINE_WALL - $CURRENT_WALL) / $BASELINE_WALL) * 100" | bc 2>/dev/null || echo "0")
    MEM_IMPROVEMENT=$(echo "scale=2; (($BASELINE_MEM - $CURRENT_MEM) / $BASELINE_MEM) * 100" | bc 2>/dev/null || echo "0")
    
    echo "IMPROVEMENTS:"
    echo "  CPU Time:  ${CPU_IMPROVEMENT}%"
    echo "  Wall Time: ${WALL_IMPROVEMENT}%"
    echo "  Memory:    ${MEM_IMPROVEMENT}%"
fi

echo ""
echo "════════════════════════════════════════════════════════════"
echo "  DETAILED PHASE BREAKDOWN"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "BASELINE:"
cat "$BASELINE"
echo ""
echo "CURRENT:"
cat "$CURRENT"
echo ""
echo "════════════════════════════════════════════════════════════"
echo "💡 To update baseline: cp $CURRENT $BASELINE"
echo "════════════════════════════════════════════════════════════"
