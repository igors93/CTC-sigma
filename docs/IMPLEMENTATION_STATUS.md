# Implementation Status

| Module | Status | Notes |
|---|---|---|
| Field arithmetic | Implemented | Canonical values in `[0, q-1]` |
| SHAKE256 constant derivation | Implemented | Cross-checked against Python `hashlib` |
| Dickson 23/47 | Implemented | Direct recurrence for forward evaluation |
| S-box | Implemented | Forward and inverse |
| Cauchy MDS | Implemented | Matrix generation and Gaussian inverse |
| ARITH | Implemented | Forward and inverse |
| Lehmer rank/unrank | Implemented | Exhaustive Python test over all 40320 ranks |
| Factor encoder | Implemented | Exact rejection rules from v0.1 |
| Simple-factor permutation | Implemented | Lehmer unrank representation |
| Left Garside normal form | Open | Explicit `CTC_STATUS_NOT_IMPLEMENTED` |
| Normal-form tokenization | Implemented | Drop/Keep tags and metadata |
| FOLD_NF | Implemented | Reference-oriented initial interpretation |
| Branch function | Wired | Operational when a normalizer is supplied |
| Feistel permutation/inverse | Wired | Operational when a normalizer is supplied |
| Sponge padding | Implemented | Unit-tested at boundary lengths |
| Hash256/XOF | Wired | Default path waits for exact normalizer |
| Frozen KAT vectors | Pending | Must follow completion of Garside normalization |

## Next implementation milestone

1. Implement exact signed-simple-factor multiplication and left Garside normalization for `B_8`.
2. Compare the C output with an independent SageMath reference.
3. Freeze constants and known-answer vectors.
4. Enable default permutation, Hash256, and XOF integration tests.
5. Add reduced-round and toy-instance instrumentation without changing production APIs.
