# Specification Traceability

This file maps the consolidated CTC-Sigma v0.2 specification to the source
tree and its validation.

| Specification area | Implementation files | Validation |
|---|---|---|
| Field `F_q` | `src/field.c`, `field.h` | field unit tests |
| Lehmer numbering | `src/lehmer.c`, `src/braid.c` | exhaustive 40320-rank test |
| Legacy public constants | `src/constants.c` | Python SHAKE256 comparison |
| v0.2 `SeedEnc` / `ConstEnc` | `src/constants.c`, `constants.h` | independent Python encoding, manifest KAT |
| Dickson, inversion, S-box | `src/field.c`, `src/arith.c` | forward/inverse tests |
| Cauchy MDS | `src/arith.c` | matrix times inverse identity |
| Generic `ARITH` | `src/arith.c` | forward/inverse regression |
| Tweakable `A_ENC_{i,h}` | `src/arith.c`, `src/encoder.c` | per-block round-trip and block KATs |
| Removal of counter alias | `src/encoder.c` | direct `u+e_0,h` versus `u,h+1` regression |
| Rejection and signed factors | `src/encoder.c` | deterministic stream and factor KAT |
| Garside normalization | `src/braid.c` | independent Python reference |
| Canonical normal-form validation | `src/braid.c`, `src/branch.c`, `src/fold.c` | negative identity/Delta/adjacency tests |
| Tokens and `FOLD_NF` | `src/fold.c` | partition, deterministic fold, negative forms |
| Branch post-mix | `src/branch.c` | default and injected-normalizer tests |
| Feistel permutation | `src/permutation.c` | forward/inverse and KATs |
| IV, encoding, padding | `src/sponge.c` | boundary-length tests |
| Absorb, squeeze, Hash/XOF | `src/sponge.c` | default tests and v0.2 KATs |
| Frozen constants and vectors | `scripts/generate_kat.py`, `test/vectors/` | JSON SHA-256 and runtime comparison |

The architecture-only injected normalizers return the canonical identity form
to isolate Feistel/sponge plumbing. They intentionally do not claim semantic
equivalence to the input word. The independent semantic cross-check is
`garside_reference.py`.
