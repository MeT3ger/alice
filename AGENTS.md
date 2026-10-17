# Agent Guidelines for ALICE Repository

This file provides architectural context, commands, and conventions for AI agents working in this repository.

---

## 1. Project Overview

**ALICE** (Application-Level Intelligent Crash Explorer) is a research tool for finding crash consistency vulnerabilities in application-level file I/O operations (power cuts, kernel panic, unflushed write buffers).

This repository is a modern fork of [`divergentdave/alice`](https://github.com/divergentdave/alice), maintained with:
1. **Linux ARM64 / AArch64 support:** Multi-architecture stack unwinding via `libunwind` in `alice-strace/syscall.c`.
2. **64-bit I/O support:** Normalization of `pwrite64 -> pwrite` in `_aliceparsesyscalls.py` for modern asynchronous runtimes like `libuv`.
3. **Visual dependency graph (`alice-graph`):** Generation of Mermaid (`.mmd`) and Graphviz (`.dot`) diagrams showing ordering, atomicity, and missing fsync barriers.
4. **Self-contained Docker environment:** Full build and execution inside Ubuntu 20.04.

---

## 2. Directory Structure

* **`alice-strace/`**: Modified `strace` tracer. Uses `libunwind` on ARM64 and low-level disassembly on x86.
* **`bin/`**: CLI utilities:
  * `alice-record`: Records system calls of an application workload.
  * `alice-check`: Explores simulated crash states and invokes a user checker script.
  * `alice-graph`: Generates Mermaid and Graphviz dependency diagrams.
* **`example/`**: Verification workloads:
  * `toy/`: Minimal original ALICE C example.
  * `libuv/`: Async I/O key-value store update via `libuv`.
  * `filesystem/`: C++17 `std::ofstream` + `std::filesystem::rename` crash test.
  * `run_all.sh`: Top-level runner for all modern examples.
* **`Dockerfile` & `.dockerignore`**: Container build configuration.
* **`.clangd` & `compile_commands.json`**: C/C++ language server and linter configuration.

---

## 3. Build & Execution Rules

### Always Run in Docker
ALICE requires `ptrace` system calls, `Python 2.7`, and specific Linux headers. Always test workloads using the container:

```bash
# 1. Build the image
docker build -t alice .

# 2. Run all example workloads
docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
  -v $(pwd):/alice \
  alice bash example/run_all.sh

# 3. Run a specific workload
docker run --rm --cap-add=SYS_PTRACE --security-opt seccomp=unconfined \
  -v $(pwd):/alice \
  alice bash example/libuv/uv_workload.sh
```

> **Note:** The flags `--cap-add=SYS_PTRACE --security-opt seccomp=unconfined` are strictly required for `strace` to intercept system calls inside the container.

---

## 4. Pattern for Adding New Examples

When adding an example under `example/<name>/`:
1. **`<name>_app.c` / `<name>_app.cpp`**:
   - Provide two execution modes via `argv[1]`: `"flawed"` (omits fsync / barrier) and `"safe"` (includes `fdatasync` and parent directory `fsync`).
   - Print a distinct commit token to stdout (e.g. `MYAPP_COMMIT_OK`) and `fflush(stdout)`.
2. **`<name>_checker.sh`**:
   - Accepts two arguments: `$1` (crashed workload directory), `$2` (captured stdout file).
   - If stdout contains the commit token, assert that the target file exists, is non-empty, and contains the updated data.
   - Return exit code `0` on pass, `1` on failure.
3. **`<name>_workload.sh`**:
   - Compiles the application.
   - Runs `alice-record --workload_dir ... --traces_dir ...`.
   - Runs `alice-check --traces_dir ... --checker ...`.
   - Runs `alice-graph --traces_dir ... --checker ... --output ...`.
4. **`.gitignore`**:
   - Ignore build artifacts, `workload_*`, `traces_*`, `graph_*`, and temporary files.

---

## 5. Development Conventions

* **Python Runtime:** ALICE internal scripts (`_aliceparsesyscalls.py`, `_aliceautotest.py`, etc.) are written in **Python 2.7**. Do not introduce Python 3-only syntax to core ALICE files unless undertaking a full Python 2-to-3 migration.
* **C/C++ Standards:**
  * C components: C11 standard.
  * C++ components: C++17 standard.
  * In macOS development, note that `fdatasync` is not exposed in `<unistd.h>` on Darwin without fallback. Use `#ifdef __APPLE__ #define fdatasync(fd) fsync(fd) #endif` for local IDE compatibility.
* **Git Remote Management:**
  * `origin`: User's fork (`https://github.com/MeT3ger/alice.git`).
  * `upstream`: Original repository (`https://github.com/divergentdave/alice.git`).
  * Default working branch: `master` tracking `origin/master`.
