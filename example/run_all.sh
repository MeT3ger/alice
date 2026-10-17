#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=========================================================="
echo "  Running ALICE Examples (libuv and std::filesystem)"
echo "=========================================================="

echo ""
echo ">>> 1. Running libuv example..."
bash "$DIR/libuv/uv_workload.sh"

echo ""
echo ">>> 2. Running std::filesystem example..."
bash "$DIR/filesystem/fs_workload.sh"

echo ""
echo "=========================================================="
echo "  All examples finished successfully!"
echo "=========================================================="
