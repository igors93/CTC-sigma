# Architecture

## Dependency direction

The implementation follows a one-way dependency structure:

```text
field ───────────────┐
keccak -> constants ─┼-> arith -> encoder ─┐
lehmer -> braid ────────────────────────────┼-> branch -> permutation -> sponge
                           fold <───────────┘
```

Lower-level modules do not depend on high-level modes. This makes each mathematical component independently testable and replaceable.

## Public API

Headers under `include/ctc_sigma/` form the supported C interface. Private helpers remain under `src/internal/`.

## Core modules

### `field`

Canonical arithmetic over `F_q`, where `q = 2^61 - 1`. Multiplication uses an unsigned 128-bit intermediate and reduces modulo `q`.

### `constants`

Implements the specification formula:

```text
Seed(label, a, b) = ASCII("CTC-SIGMA-v0.1|") || label || LE32(a) || LE32(b)
Const(label, a, b) = IntegerLE(SHAKE256(Seed, 16 bytes)) mod q
```

### `arith`

Contains Dickson/inversion S-boxes, the Cauchy MDS matrix, `ARITH`, and its inverse. The initial implementation uses the following deterministic packing for the two public constant indices:

```text
a = round_index[15:0] || subround_index[15:0]
b = lane_index[23:0] || purpose_index[7:0]
```

This mapping is localized in `src/arith.c`, so later constant-table freezing does not affect callers.

### `lehmer`

Canonical lexicographic rank and unrank for permutations of eight symbols. The implementation uses the `0..7` convention internally.

### `encoder`

Implements `v_h = A_ENC_i(u + h·e_0)`, exact 32-bit rejection, sign extraction, and rejection into the 40319 non-identity simple factors.

### `braid`

Defines signed factor and normal-form representations. The exact `B_8` left Garside normalizer is isolated behind `ctc_braid_normalizer_fn`.

### `fold`

Builds tagged tokens for version, Feistel round, ZigZag infimum, canonical length, Drop length, Drop factors, and Keep factors. It then runs the vector `FOLD_NF` accumulator.

### `branch`

Runs `A_PRE`, encoder, braid normalization, `FOLD_NF`, branch constants, and `A_POST`.

### `permutation`

Implements the twelve-round balanced Feistel permutation and its inverse. A normalizer can be injected for independent implementations and architecture tests.

### `sponge`

Implements IV initialization, message-length/domain encoding, padding, 40-byte absorption blocks, and the specified rejection-based squeeze.

## Error handling

Every fallible public function returns `ctc_status_t`. The first milestone uses `CTC_STATUS_NOT_IMPLEMENTED` specifically for the missing exact Garside normalizer. Safety limits in rejection loops return `CTC_STATUS_REJECTION_LIMIT`.
