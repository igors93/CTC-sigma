# Architecture Notes

This document describes implementation choices that are intentionally more
specific than the mathematical notation in the CTC-Sigma v0.2 specification.

## `constants`

Constants outside `A_ENC` preserve the historical v0.1 formula byte for byte:

```text
Seed(label, a, b) = ASCII("CTC-SIGMA-v0.1|") || label || LE32(a) || LE32(b)
Const(label, a, b) = IntegerLE(SHAKE256(Seed, 16 bytes)) mod q
```

The encoder uses a new domain with no packed or truncated indices:

```text
SeedEnc(c,i,h,s,j) = ASCII("CTC-SIGMA-v0.2|A_ENC-TWEAK|")
                     || LEN8(c) || ASCII(c)
                     || LE32(i) || LE32(h) || LE32(s) || LE32(j)
ConstEnc(c,i,h,s,j) = IntegerLE(SHAKE256(SeedEnc, 16 bytes)) mod q
```

The canonical component identifiers are `RC`, `SBOX-A`, `SBOX-B`, and
`SBOX-C`. The public API exposes an enum rather than accepting arbitrary
strings, so unsupported components cannot silently create new domains.
`SBOX-B` maps zero to one because it is multiplicative.

## `arith`

The arithmetic implementation has one shared forward/inverse core driven by a
constant-provider callback:

- the standard provider preserves `ARITH(label, round, x, rho)` and the v0.1
  packed-index mapping used by `A_PRE`, `A_FOLD`, and `A_POST`;
- the encoder provider derives all four lane constants from the complete
  `(round, block, subround, lane)` tuple.

This arrangement keeps the S-box, MDS multiplication, degree schedule, and
inverse logic in one implementation while changing only constant derivation.
For every fixed `(round, block)`, `A_ENC_{round,block}` remains a permutation.

## `encoder`

The encoder implements:

```text
v_h = A_ENC_{i,h}(u)
```

The input `u` is copied unchanged before the first subround. The block index is
never added, multiplied, XORed, or masked into a data lane. This removes the
v0.1 identity:

```text
A_ENC_i((u + e_0) + h*e_0) = A_ENC_i(u + (h+1)*e_0)
```

`ctc_encoder_generate_block` is a narrow public primitive used by the factor
loop, KAT generation, inverse tests, and structural regression tests.

## `braid` and injected normalizers

A canonical left normal form stores only proper simple factors:

```text
1 <= factor <= 40318
```

Identity rank `0` is omitted and Delta rank `40319` is absorbed into the
infimum. `ctc_braid_validate_normal_form` also verifies that every adjacent
pair is left-weighted. The branch validates every injected normalizer result
before FOLD, and FOLD repeats validation at its own public boundary.

This check validates representation only. It cannot prove that an external
normalizer returned a form equivalent to the signed input word, so injected
normalizers remain trusted components and must be independently tested.

## `lehmer`

Canonical lexicographic rank and unrank use permutations of symbols `0..7`.

## `fold`

The token format and arithmetic are unchanged from v0.1. The version token
continues to identify token-encoding version 1. The new validation prevents
identity, Delta, and non-left-weighted lists from entering the token stream.

## `branch`, `permutation`, and `sponge`

The branch order remains:

```text
A_PRE -> tweakable encoder -> Garside normal form -> FOLD_NF -> A_POST
```

The twelve-round Feistel construction, message encoding, padding, absorption,
and rejection-based squeeze are unchanged. Their outputs change because the
branch function now uses the v0.2 encoder.
