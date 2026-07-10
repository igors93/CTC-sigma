# Specification Traceability

This file maps the CTC-Sigma v0.1 document to the initial source tree.

| Specification area | Implementation files | Initial validation |
|---|---|---|
| Section 3.1 — `F_q` | `src/field.c`, `include/ctc_sigma/field.h` | `test_field.py` |
| Section 3.4 — Lehmer numbering | `src/lehmer.c`, `src/braid.c` | exhaustive `test_lehmer.py` |
| Section 5 — public constants | `src/constants.c`, `src/keccak.c` | Python SHAKE256 comparison |
| Sections 6.1–6.3 — Dickson, inversion, S-box | `src/field.c`, `src/arith.c` | recurrence and ARITH round-trip tests |
| Section 6.4 — Cauchy MDS | `src/arith.c` | matrix × inverse identity test |
| Section 6.5 — `ARITH` | `src/arith.c` | forward/inverse test |
| Sections 7.1–7.3 — pre-mix and factor encoder | `src/branch.c`, `src/encoder.c` | deterministic factor test |
| Section 7.4 — Garside normalization | `src/braid.c` | interface present; exact algorithm pending |
| Sections 7.5–7.6 — tokens and `FOLD_NF` | `src/fold.c` | Drop/Keep and deterministic fold tests |
| Section 7.7 — post-mix | `src/branch.c` | covered through injected-normalizer tests |
| Section 8 — Feistel permutation | `src/permutation.c` | forward/inverse integration test |
| Sections 9.1–9.2 — IV, encoding, padding | `src/sponge.c` | boundary-length tests |
| Sections 9.3–10.2 — absorb, squeeze, Hash/XOF | `src/sponge.c` | test-normalizer integration test |
| Sections 12–13 — modularity and vectors | whole tree, `test/python/` | initial phase only; frozen KATs pending |

The injected normalizer used by slow Python integration tests exists only to validate the Feistel and sponge plumbing. It is not compiled into the C core and must never be treated as the specified Garside algorithm.
