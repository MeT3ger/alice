#!/bin/bash
# ALICE Consistency Checker for std::filesystem example
# $1: Crashed workload directory
# $2: Captured stdout file at the crash point

WORK_DIR="$1"
STDOUT_FILE="$2"

if grep -q "STD_FS_COMMIT_OK" "$STDOUT_FILE" 2>/dev/null; then
    if [ ! -f "$WORK_DIR/data_fs.db" ]; then
        echo "FAIL: Commit acknowledged but data_fs.db does not exist!"
        exit 1
    fi
    if [ ! -s "$WORK_DIR/data_fs.db" ]; then
        echo "FAIL: Commit acknowledged but data_fs.db is empty (0 bytes)!"
        exit 1
    fi
    if ! grep -q "version 2" "$WORK_DIR/data_fs.db"; then
        echo "FAIL: Commit acknowledged but data_fs.db does not contain version 2!"
        exit 1
    fi
fi

if [ -f "$WORK_DIR/data_fs.db" ] && [ ! -s "$WORK_DIR/data_fs.db" ]; then
    echo "FAIL: data_fs.db exists but is 0 bytes (corrupted)!"
    exit 1
fi

exit 0
