# CTC-Sigma v0.1 — C Core

This repository is a research-oriented implementation of the CTC-Sigma v0.1 specification.
The cryptographic core is written in C11. Validation and development tests are written in Python with `pytest` and call the shared C library through `ctypes`.

> **Experimental status:** this project must not be used to protect real data. No security property has been proven.

## Current scope

Implemented and tested:

- arithmetic over `F_q`, with `q = 2^61 - 1`;
- Dickson polynomials of degrees 23 and 47;
- finite-field inversion with `Inv(0) = 0`;
- reproducible SHAKE256 public constants;
- lane S-boxes and their inverse;
- the 8×8 Cauchy MDS matrix and inverse;
- parameterized `ARITH` and inverse;
- Lehmer rank/unrank for `S_8`;
- unbiased factor generation described by the encoder;
- **the exact left Garside normal form for signed simple factors in `B_8`**,
  cross-checked against an independent Python reference implementation;
- canonical normal-form tokenization and `FOLD_NF`;
- Feistel branch, permutation, and inverse on the default path;
- sponge padding, absorption, and rejection-based squeeze;
- operational `CTC-Sigma-256` and `CTC-Sigma-XOF` with frozen known-answer
  vectors in `test/vectors/`.

The braid normalizer remains injectable (`ctc_braid_normalizer_fn`) so that
additional independent implementations can be compared without changing any
public interface.

## Repository layout

```text
.
├── cmake/                    Reusable CMake configuration
├── docs/                     Architecture, status, and spec traceability
├── include/ctc_sigma/        Public C API
├── src/                      C cryptographic core
│   └── internal/             Private implementation headers
├── test/
│   ├── python/               Python unit and integration tests
│   │   ├── unit/             Per-module tests
│   │   ├── integration/      Default-path, cross-implementation, KAT tests
│   │   └── garside_reference.py  Independent Garside implementation
│   └── vectors/              Frozen known-answer vectors
├── tools/                    Small development executables
└── scripts/                  Build helper and KAT generator
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

Install pytest and run the suite (the `slow` marker gates the more expensive
integration tests):

```bash
python -m pip install pytest
pytest                # everything
pytest -m "not slow"  # fast subset
```

The suite includes:

- unit tests per module, including an exhaustive Lehmer rank/unrank sweep;
- random-word agreement between the C Garside normalizer and the independent
  Python reference (`test_braid.py`);
- full-pipeline agreement of Hash256/XOF when the Python normalizer is
  injected into the C sponge (`test_cross_implementation.py`);
- comparison against the frozen vectors in
  `test/vectors/ctc_sigma_v01_kat.json` (`test_kat.py`).

## Known-answer vectors

`test/vectors/ctc_sigma_v01_kat.json` freezes Hash256 digests for message
lengths 0, 1, 39, 40, 41, 80, and 1024, XOF outputs of 1, 31, 32, 40, 41, 64,
and 1000 bytes for the empty and a fixed message, and permutation vectors.
Messages follow the documented convention `byte[i] = i mod 256`. Regenerate
(only after an intentional, documented specification change) with:

```bash
python scripts/generate_kat.py
```

## Design rules

- C source code, identifiers, and comments use English.
- Modules expose narrow public interfaces and avoid cross-layer dependencies.
- Errors are returned through `ctc_status_t`; cryptographic functions do not terminate the process.
- No floating-point arithmetic is used.
- External words are little-endian.
- The permutation-braid convention is documented in `src/braid.c` and mirrored by the Python reference.
- The code favors reference clarity and testability over performance in this milestone.
