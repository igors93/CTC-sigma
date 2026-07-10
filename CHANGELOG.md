# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added — continuous integration (2026-07-10)

- GitHub Actions workflow (`.github/workflows/ci.yml`) with two jobs:
  - **Build and test** on GCC (warnings promoted to errors) and Clang:
    Release build, constant-dump smoke test, full pytest suite, and a check
    that `scripts/generate_kat.py` reproduces the frozen KAT vectors bit for
    bit;
  - **AddressSanitizer / UBSan**: Debug build with
    `-fsanitize=address,undefined`, running the full suite with `libasan`
    preloaded into the Python interpreter.

### Changed — README.md visual header and diagrams (2026-07-10)

- Added a centered header with status badges (CI, language, build system,
  test framework, specification version, experimental status, security
  disclaimer) linked to the GitHub Actions workflow.
- Replaced the three ASCII diagrams with GitHub-rendered Mermaid flowcharts:
  the sponge data flow, the six-stage branch function `F_i` pipeline (now
  showing the Drop/Keep split and the `u` feed-forward into A_POST), and the
  module dependency architecture.

### Changed — README.md rewritten (2026-07-10)

The README was rewritten from scratch as a professional, English-language
project document derived from the CTC-Σ v0.1 specification:

- added a specification-driven explanation of the design ideas: sponge mode,
  the 12-round Feistel permutation PΣ, the branch function `F_i` pipeline
  (A_PRE → expander → signed factors → Garside normal form → Drop/Keep →
  FOLD_NF → A_POST), the arithmetic layer (Dickson/inversion S-boxes, Cauchy
  MDS), the braid layer, and SHAKE256 constant derivation;
- added a "Design at a glance" parameter table matching Section 4 of the
  specification;
- added a "Security goals and non-claims" section reproducing the
  specification's goal-versus-proof distinction;
- added an "Implementation architecture" section with the module dependency
  diagram and a per-module responsibility table;
- added a table of contents, build artifact table, four-layer testing
  description, KAT documentation, and a roadmap mapped to the
  specification's Phases 0Σ/1Σ/2Σ;
- previous content describing the current scope, layout, build, and design
  rules was merged into the new structure.

## [0.1.0] — 2026-07-10

First feature-complete milestone of the CTC-Σ v0.1 specification.

### Added

- Exact left Garside normal form for signed simple factors of `B₈` in
  `src/braid.c` (permutation-braid representation, negative-factor rewriting
  via `x⁻¹ = Δ⁻¹·τ(∂(x))`, meet-based left-weighting to a fixed point),
  completing the last open module and enabling the default permutation,
  Hash256, and XOF paths.
- Independent Python reference implementation of the Garside normal form
  (`test/python/garside_reference.py`) using atom-transfer left-weighting,
  fulfilling the specification's two-independent-implementations
  requirement (Section 12.4).
- Cross-validation tests: random-word agreement between C and Python
  normalizers, left-weighted output verification, `S₈` projection checks
  (`test/python/unit/test_braid.py`), and byte-exact full-pipeline agreement
  of Hash256/XOF with the Python normalizer injected
  (`test/python/integration/test_cross_implementation.py`).
- Frozen known-answer vectors (`test/vectors/ctc_sigma_v01_kat.json`)
  covering the message and output lengths mandated by Section 13, plus
  permutation vectors; generator script `scripts/generate_kat.py` and
  verification test `test/python/integration/test_kat.py`.
- Default-path integration tests (permutation round-trip, deterministic
  hashing, XOF prefix consistency, domain separation, block-boundary
  lengths) in `test/python/integration/test_default_paths.py`.
- Python bindings for `ctc_braid_normalize_left`, `ctc_hash256`, `ctc_xof`,
  and `ctc_permutation_inverse`, with `hash256()`/`xof()` helpers.
- `.gitignore` for build output, Python bytecode, and tool caches.
- This changelog.

### Changed

- `include/ctc_sigma/braid.h`: documentation updated from
  "intentionally not implemented" to the description of the implemented
  normal form and its injectable-normalizer contract.
- `docs/IMPLEMENTATION_STATUS.md`, `docs/ARCHITECTURE.md`, and
  `docs/SPEC_TRACEABILITY.md` updated to reflect the completed braid module,
  the cross-implementation validation strategy, and the frozen vectors.

### Removed

- `test/python/integration/test_initial_status.py`, which asserted that the
  default permutation reports `CTC_STATUS_NOT_IMPLEMENTED`; superseded by
  the default-path tests.
- Build artifacts (`build/`) and Python bytecode caches from version
  control; they are now ignored and regenerated locally.

### Fixed

- Stale CMake cache referencing an unavailable `gmake` binary; the build
  tree is regenerated with the system `make` and now exports
  `compile_commands.json` for IDE tooling.

## [0.1.0-dev] — initial import

- C11 core: field arithmetic over `q = 2⁶¹ − 1`, SHAKE256 constant
  derivation, Dickson 23/47 S-boxes with inverses, Cauchy MDS matrix and
  inverse, parameterized `ARITH`, Lehmer rank/unrank for `S₈`, bias-free
  factor encoder, normal-form tokenization and `FOLD_NF`, Feistel branch and
  permutation with injectable braid normalizer, sponge with injective
  padding and rejection-based squeeze.
- Python `pytest` suite driving the shared library through `ctypes`.
- The exact Garside normalizer intentionally returned
  `CTC_STATUS_NOT_IMPLEMENTED` pending the 0.1.0 milestone.
