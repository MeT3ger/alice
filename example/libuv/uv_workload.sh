#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

chmod +x uv_checker.sh

echo "=== [libuv Example] 1. Compiling application ==="
gcc -g -O0 uv_app.c -luv -o uv_app

run_mode() {
    local MODE="$1"
    local WL="workload_${MODE}"
    local TR="traces_${MODE}"
    local GRAPH="graph_uv_${MODE}"

    echo ""
    echo "=== [libuv Example] Running in [$MODE] mode ==="
    rm -rf "$WL" "$TR"
    mkdir -p "$WL" "$TR"
    echo -e "version: 1 (initial)\n" > "$WL/data_uv.db"

    (
        cd "$WL"
        alice-record --workload_dir . --traces_dir "../$TR" ../uv_app "$MODE"
    )

    echo "--- Checking crash consistency with alice-check ---"
    alice-check --traces_dir "$TR" --checker "$DIR/uv_checker.sh" || true

    echo "--- Generating visual graph with alice-graph ---"
    alice-graph --traces_dir "$TR" --checker "$DIR/uv_checker.sh" --output "$GRAPH" || true
}

run_mode "flawed"
run_mode "safe"
