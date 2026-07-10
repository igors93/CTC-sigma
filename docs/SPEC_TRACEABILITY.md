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
| Section 7.4 — Garside normalization | `src/braid.c` | cross-checked against `garside_reference.py` in `test_braid.py` |
| Sections 7.5–7.6 — tokens and `FOLD_NF` | `src/fold.c` | Drop/Keep and deterministic fold tests |
| Section 7.7 — post-mix | `src/branch.c` | covered through injected-normalizer tests |
| Section 8 — Feistel permutation | `src/permutation.c` | forward/inverse integration test |
| Sections 9.1–9.2 — IV, encoding, padding | `src/sponge.c` | boundary-length tests |
| Sections 9.3–10.2 — absorb, squeeze, Hash/XOF | `src/sponge.c` | default-path tests and frozen KAT comparison |
| Section 12.4 — two independent implementations | `src/braid.c`, `test/python/garside_reference.py` | random-word and full-pipeline agreement tests |
| Sections 12–13 — modularity and vectors | whole tree, `test/vectors/` | KAT vectors frozen in `ctc_sigma_v01_kat.json` |

The simplistic normalizer injected by `test_feistel_with_test_normalizer.py` and `test_sponge_with_test_normalizer.py` exists only to validate the Feistel and sponge plumbing independently of the braid layer. It is not compiled into the C core and is not a Garside algorithm. The Garside cross-check lives in `test_braid.py` and `test_cross_implementation.py`.
