#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

chmod +x fs_checker.sh

echo "=== [std::filesystem Example] 1. Compiling application ==="
g++ -std=c++17 -g -O0 fs_app.cpp -o fs_app

run_mode() {
    local MODE="$1"
    local WL="workload_${MODE}"
    local TR="traces_${MODE}"
    local GRAPH="graph_fs_${MODE}"

    echo ""
    echo "=== [std::filesystem Example] Running in [$MODE] mode ==="
    rm -rf "$WL" "$TR"
    mkdir -p "$WL" "$TR"
    echo -e "version: 1 (initial)\n" > "$WL/data_fs.db"

    (
        cd "$WL"
        alice-record --workload_dir . --traces_dir "../$TR" ../fs_app "$MODE"
    )

    echo "--- Checking crash consistency with alice-check ---"
    alice-check --traces_dir "$TR" --checker "$DIR/fs_checker.sh" || true

    echo "--- Generating visual graph with alice-graph ---"
    alice-graph --traces_dir "$TR" --checker "$DIR/fs_checker.sh" --output "$GRAPH" || true
}

run_mode "flawed"
run_mode "safe"
