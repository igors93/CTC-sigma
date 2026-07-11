# Implementation Status

| Module | Status | Notes |
|---|---|---|
| Field arithmetic | Implemented | Canonical values in `[0, q-1]` |
| Legacy SHAKE256 constants | Preserved | Exact `CTC-SIGMA-v0.1|` domain |
| Encoder tweak constants | Implemented | Exact v0.2 tuple, no index packing or truncation |
| Dickson 23/47 | Implemented | Forward and inverse through permutation degrees |
| S-box | Implemented | Shared forward/inverse core |
| Cauchy MDS | Implemented | Matrix generation and Gaussian inverse |
| Generic ARITH | Implemented | Forward and inverse; historical domains preserved |
| Tweakable A_ENC | Implemented | Forward/inverse per `(round, block)` |
| Lehmer rank/unrank | Implemented | Exhaustive test over all 40320 ranks |
| Factor encoder | Implemented | `v_h=A_ENC_{i,h}(u)`, exact rejection rules |
| Encoder alias regression | Implemented | Tests all 12 rounds and adjacent blocks |
| Left Garside normal form | Implemented | Cross-checked against independent Python reference |
| Normal-form validation | Implemented | Proper factors and left-weighted adjacency |
| Normal-form tokenization | Implemented | Rejects noncanonical external forms |
| FOLD_NF | Implemented | Algorithm unchanged from v0.1 |
| Branch function | Implemented | Validates injected normalizer output before FOLD |
| Feistel permutation/inverse | Implemented | Round-trip tested on default and injected paths |
| Sponge padding | Implemented | Unit-tested at boundary lengths |
| Hash256/XOF | Implemented | v0.2 default paths operational |
| Frozen v0.2 KAT vectors | Frozen | Hash, XOF, permutation, encoder blocks and factors |
| Encoder constant manifest | Frozen | JSON manifest plus SHA-256 checksum |

## Cross-implementation validation

Two independent normal-form implementations are compared by the test suite:

1. `src/braid.c` — meet-based left-weighting over permutation braids;
2. `test/python/garside_reference.py` — atom-transfer left-weighting using
   starting/finishing sets, written without reference to the C code path.

The C/Python normalizers must agree on random signed words and through the full
Hash256/XOF pipeline. Independently encoded Python SHAKE256 seeds also verify
the new encoder constant domain.

## Compatibility

The v0.2 encoder changes the branch function and therefore changes P-Sigma,
Hash256, and XOF outputs. `ctc_sigma_v01_kat.json` is retained only as a
historical snapshot. The active suite uses `ctc_sigma_v02_kat.json` and the
v0.2 encoder constant manifest.

## Next research milestone

1. Repeat the original encoder correlation and alignment studies against v0.2.
2. Search for constant and simple affine relations between different blocks.
3. Measure rejection and factor distributions independently for each block.
4. Propagate chosen `A_PRE^-1` pairs through Garside, FOLD, branch, and reduced
   Feistel rounds.
5. Obtain an independent implementation of the complete v0.2 constant and
   encoder path before treating the KAT set as externally reproduced.
