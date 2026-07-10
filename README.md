# CTC-Sigma v0.1 — Initial C Core

This repository is an initial, research-oriented implementation of the CTC-Sigma v0.1 specification.
The cryptographic core is written in C11. Validation and development tests are written in Python with `pytest` and call the shared C library through `ctypes`.

> **Experimental status:** this project must not be used to protect real data. No security property has been proven.

## Current scope

Implemented in this first milestone:

- arithmetic over `F_q`, with `q = 2^61 - 1`;
- Dickson polynomials of degrees 23 and 47;
- finite-field inversion with `Inv(0) = 0`;
- reproducible SHAKE256 public constants;
- lane S-boxes and their inverse;
- the 8×8 Cauchy MDS matrix and inverse;
- parameterized `ARITH` and inverse;
- Lehmer rank/unrank for `S_8`;
- unbiased factor generation described by the encoder;
- canonical normal-form tokenization and `FOLD_NF`;
- Feistel branch, permutation, inverse, sponge padding, absorption, and squeeze architecture;
- injectable braid-normalizer interface for independent implementations and testing.

The exact left Garside normal form for signed simple factors in `B_8` remains intentionally explicit as `CTC_STATUS_NOT_IMPLEMENTED`. The repository does not silently replace it with a different braid reduction. Once that module is completed, the default permutation, Hash256, and XOF paths become operational without changing their public interfaces.

## Repository layout

```text
.
├── .github/workflows/       Continuous integration and CodeQL
├── cmake/                   Reusable CMake configuration
├── docs/                    Architecture and implementation status
├── include/ctc_sigma/       Public C API
├── src/                     C cryptographic core
│   └── internal/            Private implementation headers
├── test/python/             Python unit and integration tests
├── tools/                   Small development executables
└── scripts/                 Local build helpers
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The build produces:

- `build/libctc_sigma.a` — static library;
- `build/libctc_sigma.so` — shared library used by Python tests;
- `build/ctc_dump_constants` — public constant preview tool.

## Run tests

Install pytest and run the initial suite:

```bash
python -m pip install pytest
pytest -m "not slow"
```

The slow Feistel round-trip test injects a test-only normalizer to validate the surrounding architecture. It is not a Garside implementation:

```bash
pytest -m slow
```

## Design rules

- C source code, identifiers, and comments use English.
- Modules expose narrow public interfaces and avoid cross-layer dependencies.
- Errors are returned through `ctc_status_t`; cryptographic functions do not terminate the process.
- No floating-point arithmetic is used.
- External words are little-endian.
- Incomplete mathematical functionality returns an explicit status rather than producing a non-specification result.
- The code favors reference clarity and testability over performance in this milestone.
# CTC-Sigma
