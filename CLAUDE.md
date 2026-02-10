# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OpenMP-based parallel triangle counting for sparse graphs (C99). The binary is called `gktc`. Implements the hash-map-based JIK algorithm from the GraphChallenge 2017 finalist paper by Tom et al.

## Build Commands

**Dependency:** [GKlib](https://github.com/KarypisLab/GKlib) must be built and installed first (defaults to `~/local`).

```bash
make config                          # configure (runs CMake under build/$(uname -s)-$(uname -m))
make config gklib_path=/path/to/gk   # if GKlib is not in ~/local
make config cc=icc                   # use Intel compiler
make config openmp=not-set           # build serial version (no OpenMP)
make                                 # build
make install                         # install to ~/local/bin by default
make clean                           # remove object files
make distclean                       # remove entire build directory
```

Build output goes to `build/<OS>-<arch>/` (e.g., `build/Darwin-arm64/`).

## Running

```bash
gktc [options] infile
gktc -nthreads=4 test/p2p-Gnutella31.metis    # sample run with test data
```

Input formats: Metis (default) or TSV (`-iftype=tsv`). Graphs must be undirected with both edge directions present.

## Architecture

The codebase is small (~775 lines across 4 source files):

- **`main.c`** — Entry point: parses args, loads graph, runs algorithm, prints results.
- **`cmdline.c`** — Command-line parsing via GKlib's option parser.
- **`io.c`** — Graph loading from Metis or TSV format into `gk_graph_t`.
- **`ptc.c`** — Core algorithm in two phases:
  1. **`ptc_Preprocess`**: Reorders vertices by degree, sorts adjacency lists, splits into lower/upper triangular parts.
  2. **`ptc_MapJIK`**: Counts triangles using hash-map-based adjacency intersection with two kernels — one for general vertices (hash probing with collisions) and one for high-index vertices (direct array indexing).

Headers: `tc.h` (master include), `struct.h` (defines `vault_t` and `params_t`), `defs.h` (constants like `HTABLE_*`, `LNBITS`), `proto.h` (prototypes).

## Key Implementation Details

- Parallelism via OpenMP with dynamic scheduling and reduction clauses
- Hardware prefetching (`__builtin_prefetch`) for cache optimization
- Optional AVX-512 vectorization controlled by `TC_VECOPT` preprocessor flag
- Hash table sizing uses power-of-2 with configurable load factors (`HTABLE_LIMIT_*` in `defs.h`)
- Compiler flags configured in `conf/gkbuild.cmake` — uses `-O3 -flto` for GCC, architecture-specific tuning for x86_64 vs ARM64
