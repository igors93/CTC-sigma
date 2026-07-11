# CTC-Σ v0.2 — Audit Findings, Confirmation Record, and Correction Considerations

**Document status:** Technical audit record and design-review input  
**Project:** CTC-Σ  
**Version evaluated:** `0.2.0-dev`  
**Primary commit evaluated:** `f7eaffdd76140803b5796ab9c51955bdb0fb8e86`  
**Evaluation date:** 11 July 2026  
**Purpose:** Consolidate the implementation, API-contract, and sponge-claim findings observed by the v0.2 audit suites, distinguish confirmed results from inconclusive observations, and define the correction decisions that must be considered before promoting `0.2.0-dev` to a stable release.

> **Security notice**
>
> CTC-Σ remains an experimental cryptographic construction. The findings in this document do not demonstrate a collision, preimage, second-preimage, or practical distinguisher against CTC-Σ Hash256 or CTC-Σ XOF. They identify two confirmed public-API contract defects and one unresolved theoretical claim issue. Passing later regression tests will not constitute a proof of cryptographic security or authorize production use.

---

## 1. Intended use of this document

This document is a correction-planning record.

It is intended to help maintainers decide:

- which behaviors are implementation defects;
- which behaviors are mathematically valid but inconsistent with the public contract;
- which security statements are unsupported by the currently documented model;
- which changes can preserve current known-answer vectors;
- which changes require a new incompatible specification revision;
- which tests must be added before the next release.

This document does not itself change the CTC-Σ specification. A correction becomes normative only after the relevant source code, public headers, tests, known-answer vectors, changelog, implementation-status record, and technical specification have been updated consistently.

Raw result archives remain the source of truth for the individual executions.

---

## 2. Result vocabulary

The following terms have precise meanings throughout this document.

### CONFIRMED

A deterministic behavior was reproduced with sufficient coverage, appropriate controls, and direct agreement with the relevant implementation logic.

### CONTRACT MISMATCH CONFIRMED

The implementation behavior was reproduced, and the behavior conflicts with the current public documentation or implied API contract.

This classification does not automatically mean that the underlying mathematics is incorrect.

### MODEL-DEPENDENT REVIEW

The parameter arithmetic is correct under a stated model, but the model has not been formally shown to apply unchanged to the complete CTC-Σ construction.

### NOT CONFIRMED

The tested evidence was insufficient to establish the proposed relation, weakness, contribution, or security effect.

### BLOCKED

A test component did not execute because of a test-harness or environment problem rather than a confirmed CTC-Σ implementation failure.

### SUPERSEDED

A preliminary result has been replaced by a larger or more targeted experiment.

---

## 3. Evidence base

Three result packages are consolidated here.

| Run | Purpose | Duration | Principal role |
|---|---|---:|---|
| `20260711T200828Z-quick-audit-v02` | Fast observational audit | 8.861 s | Initial detection and screening |
| `20260711T202515Z-confirmatory-issues-v02` | High-volume confirmation of the three selected issues | 7.424 s | Main confirmatory evidence |
| `20260711T210229Z-targeted-confirmation-v02` | Independent targeted repetition and capacity-model review | 1.941 s | Reconfirmation and refinement of interpretation |

### 3.1 Archive integrity

```text
Quick-audit ZIP SHA-256:
5837188620563000381ae45ade4e1514556ebcf6111c757af026a465bf817bb9

Confirmatory-issues ZIP SHA-256:
43366fe8f46a4a381b84d4db76949ab03549235bffe99ecf8a1204d5850d6850

Targeted-confirmation ZIP SHA-256:
66e36f37989db92b3ac55fd38214630189a07d4e3a6e1a40bf6aa0129589da3
```

Manifest verification reported:

```text
Quick audit:             13 of 13 files valid
Confirmatory issues:     24 of 24 files valid
Targeted confirmation:   10 of 10 files valid
Manifest mismatches:      0
```

### 3.2 Common implementation provenance

The executions identified the same core revision and shared-library image:

```text
Commit:
f7eaffdd76140803b5796ab9c51955bdb0fb8e86

Library version:
0.2.0-dev

Shared-library SHA-256:
223bbf06f259ab3d0b5a75999d3aa47ef5acd5df1e644e92534b694caf861733
```

Recorded environment:

```text
Operating system: Linux 7.0.14-101.fc43.x86_64
Architecture:     x86_64
Python:           CPython 3.14.6
```

The repeated library hash materially strengthens the comparison between runs: the observations were not produced by different CTC-Σ binaries.

### 3.3 Evidence limitations

The result archives contain reports, raw observations, manifests, environment data, and source excerpts. Complete scientific reproducibility also requires preserving the exact Python harness and native probe sources under `research/`.

The evidence was generated on one principal operating system and architecture. Cross-platform confirmation remains desirable.

---

## 4. Executive summary

| Identifier | Finding | Final classification | Direct Hash256/XOF break demonstrated? |
|---|---|---|---:|
| `F-API-001` | Legacy generic ARITH indices alias modulo $2^{16}$ | **CONFIRMED implementation/API defect** | No |
| `F-API-002` | Public low-level APIs silently accept noncanonical field representatives | **BEHAVIOR CONFIRMED; CONTRACT MISMATCH CONFIRMED** | No |
| `F-SPEC-001` | The documented 256-bit preimage and second-preimage goals are not justified by the conventional declared-capacity model | **MODEL-DEPENDENT THEORETICAL REVIEW** | No |
| `O-ENC-001` | The removed v0.1 input/counter identity did not reappear in the tested v0.2 encoder relations | **NOT OBSERVED** | Not applicable |
| `O-SBOX-001` | Exact central S-box symmetry exists locally | **CONFIRMED LOCALLY; DOWNSTREAM IMPACT NOT CONFIRMED** | No |
| `O-BRAID-001` | Full and braid-knockout avalanche means were similar in a very small sample | **INCONCLUSIVE** | No |
| `T-HARNESS-001` | The native probe in the targeted repetition failed to compile under `-Werror` | **BLOCKED TEST COMPONENT** | Not applicable |

The two API findings are sufficiently confirmed to justify corrective engineering work.

The sponge-capacity issue is sufficiently established to require a specification decision, but not sufficiently established to claim that the effective preimage security of CTC-Σ is exactly 244 bits.

---

# 5. F-API-001 — Legacy generic ARITH index aliasing

## 5.1 Finding statement

Distinct public round and subround identifiers separated by multiples of $2^{16}$ select identical legacy constant domains.

Representative identities include:

```text
round 0   ≡ round 65536
round 1   ≡ round 65537
round 11  ≡ round 65547

subround 0 ≡ subround 65536
subround 1 ≡ subround 65537
```

The affected behavior belongs to the legacy generic ARITH/S-box constant path used by components such as `A_PRE`, `A_FOLD`, and `A_POST`.

The v0.2 tweakable encoder constant domain is separate and is not the source of this finding.

---

## 5.2 Root cause

The legacy implementation packs the round and subround indices into one 32-bit value after truncating both to 16 bits:

```c
static uint32_t ctc_primary_index(
    uint32_t round_index,
    uint32_t subround_index
) {
    return ((round_index & UINT32_C(0xFFFF)) << 16U)
        | (subround_index & UINT32_C(0xFFFF));
}
```

Therefore:

$$
D(r,s)=D(r+k2^{16},s+m2^{16})
$$

for all representable values for which the additions are defined.

The equality is algebraic and deterministic. It is not a statistical correlation.

---

## 5.3 Observational and confirmatory evidence

### Quick audit

```text
ARITH round alias cases:             72
ARITH round equal outputs:           72

S-box subround alias cases:         192
S-box subround equal outputs:       192
```

### Main confirmatory suite

```text
ARITH alias cases:                3,000
Calls returning status OK:        3,000
Exactly equal outputs:            3,000

Neighbor controls:                  600
Neighbor controls producing
different outputs:                  600

S-box alias cases:               21,600
Calls returning status OK:       21,600
Exactly equal outputs:           21,600

Neighbor controls:                4,320
Neighbor controls producing
different outputs:                4,320

Direct constant comparisons:      9,600
Exactly equal constants:          9,600

Nominal domain groups tested:         15
Groups collapsing to one
effective domain:                     15
```

### Targeted repetition

```text
Alias cases:                         900
Alias calls returning status OK:     900
Exactly equal alias outputs:         900

Neighbor controls:                   900
Neighbor calls returning status OK:  900
Neighbor outputs different:          900
```

### Independent native evidence

The main confirmatory package successfully compiled and executed an independent C probe.

It confirmed:

```text
ARITH round 0 / 65536, forward:       equal
ARITH round 11 / 65547, inverse:      equal
S-box subround 0 / 65536, forward:    equal
S-box subround 1 / 65537, inverse:    equal
```

Every call returned `CTC_STATUS_OK`.

---

## 5.4 Why the controls matter

The neighboring-index controls produced different outputs in every recorded case.

For example, the harness distinguished:

```text
round r       from round r + 1
subround s    from subround s + 1
```

while still observing equality for:

```text
round r       and round r + 65536
subround s    and subround s + 65536
```

This rules out explanations such as:

- a comparison bug that always reports equality;
- an unchanged input buffer;
- a constant function in the tested path;
- accidental reuse of one result for both calls.

The observed equivalence matches the source-level mask exactly.

---

## 5.5 Security and engineering impact

The normative v0.2 Hash256 and XOF paths use small round and subround values. The tests did not demonstrate a digest collision, XOF collision, preimage attack, or full-construction distinguisher caused by this alias.

The confirmed impact is instead:

- distinct public identifiers can select the same constants;
- callers can believe they created separate domains when they did not;
- future parameter expansion can silently reuse existing transformations;
- research experiments using large indices can produce misleading results;
- the public `uint32_t` interface represents a larger domain than the effective implementation;
- the implementation violates the expected uniqueness of explicit domain identifiers.

This is a real API and domain-separation defect even though the currently normative path does not reach the aliased range.

---

## 5.6 What was not demonstrated

`F-API-001` does not demonstrate:

- a collision in CTC-Σ Hash256;
- a repeated constant inside the current 12-round normative schedule;
- a preimage or second-preimage shortcut;
- a practical attack on the revised encoder;
- a failure of the Feistel inverse;
- a failure of the v0.2 encoder tweak derivation.

---

## 5.7 Correction option A — strict range validation

The least invasive correction is to preserve the existing legacy derivation and reject values that would be truncated.

For direct S-box entry points:

```text
round_index    <= 65535
subround_index <= 65535
```

For generic ARITH calls, the implementation must ensure that every internally generated subround index fits in 16 bits.

A suitable contract is:

```text
invalid index → CTC_STATUS_OUT_OF_RANGE
```

### Advantages

- preserves all constants used by the current normative path;
- preserves Hash256, XOF, permutation, and current KAT outputs;
- makes the effective domain explicit;
- requires limited source changes;
- provides a clear regression target.

### Disadvantages

- retains the packed legacy derivation;
- limits future experiments to the documented range;
- keeps an implementation detail that is less clean than independent field encoding.

### Recommended use

This is the recommended immediate correction when preserving v0.2 KAT compatibility is a priority.

---

## 5.8 Correction option B — lossless constant-domain encoding

A cleaner redesign is to replace packed masked indices with independent encoded fields, for example:

```text
prefix || label || LE32(round) || LE32(subround)
       || LE32(lane) || LE32(purpose)
```

No public identifier would be truncated.

### Advantages

- removes the equivalence classes at the derivation level;
- supports the complete `uint32_t` domain;
- provides clearer domain separation;
- aligns conceptually with the v0.2 encoder tweak encoding.

### Disadvantages

- changes all affected non-encoder constants;
- changes the branch function;
- changes the permutation;
- changes Hash256 and XOF outputs;
- requires replacement KATs and specification text;
- is an incompatible algorithm revision.

### Recommended use

This option should be treated as a new incompatible specification change, even if implemented before the first stable v0.2 tag.

---

## 5.9 Required regression tests

A correction must add tests proving that:

1. every valid normative round and subround still succeeds;
2. values outside the documented range return `CTC_STATUS_OUT_OF_RANGE`;
3. rejected calls do not partially mutate the supplied state;
4. the old alias pairs no longer return two successful equal-domain calls;
5. neighboring valid indices remain distinct;
6. forward and inverse paths enforce the same limits;
7. canonical-input KATs remain unchanged under strict validation;
8. regenerated KATs are reviewed if the derivation is redesigned.

---

## 5.10 Disposition

```text
Finding ID:     F-API-001
Status:         CONFIRMED
Nature:         implementation/API/domain-separation defect
Normative path: not shown to be directly exploitable
Correction:     required before stable v0.2 release
```

---

# 6. F-API-002 — Noncanonical public-state representations

## 6.1 Finding statement

Several public low-level APIs accept multiple `uint64_t` representatives of the same field element, return `CTC_STATUS_OK`, and silently reduce them modulo:

$$
q=2^{61}-1.
$$

Thus:

$$
x,\quad x+q,\quad x+2q,\quad \ldots
$$

are treated as the same element whenever the values fit in `uint64_t`.

The implementation returns canonical outputs even when the original input representation was noncanonical.

---

## 6.2 Mathematical background

CTC-Σ operates over:

$$
\mathbb F_q.
$$

Inside the field, the equivalence:

$$
x+kq \equiv x \pmod q
$$

is mathematically correct.

Therefore, the observed equality is not evidence that field addition, multiplication, inversion, or reduction is mathematically broken.

The issue is the public representation contract.

The documentation describes externally represented field elements as canonical integers satisfying:

$$
0 \leq x < q.
$$

A public API that silently accepts $x\geq q$ implements a permissive canonicalization contract rather than a strict canonical-input contract.

---

## 6.3 Root cause

The elementary field layer reduces incoming values modulo $q$:

```c
uint64_t ctc_field_reduce_u64(uint64_t value) {
    return value % CTC_FIELD_MODULUS;
}
```

Higher public state APIs validate pointers and selected index ranges but do not consistently validate every input lane before invoking field operations.

The first field operation therefore collapses noncanonical representatives to their canonical residue.

---

## 6.4 Observational and confirmatory evidence

### Quick audit

```text
ARITH comparisons x versus x + q:          12
Exactly equal ARITH outputs:                12

Permutation comparisons:                    3
Exactly equal permutation outputs:           3

Permutation round-trips:                     3
Returned canonical input:                    3
Returned original noncanonical bits:         0
```

### Main confirmatory suite

#### Scalar S-box

```text
Cases tested:                             4,176
Calls returning status OK:               4,176
Exactly equal outputs:                   4,176
```

#### Vector APIs

```text
Cases tested:                               772
Calls returning status OK:                   772
Exactly equal outputs:                       772
Canonical outputs:                           772
```

The tested vector paths included:

- generic ARITH forward and inverse;
- encoder ARITH forward and inverse;
- raw encoder-block generation;
- branch evaluation;
- permutation forward;
- permutation inverse.

#### Factor streams

```text
Cases tested:                                30
Calls returning status OK:                   30
Exactly equal factor streams:                30
```

Equality covered:

- factor ranks;
- factor signs;
- generated block counts.

#### Round-trips

```text
Cases tested:                                30
Returned canonical representative:          30
Returned original noncanonical bits:          0
```

#### Genuine-change controls

```text
Controls using a real field change:          24
Controls producing different outputs:        24
```

The controls used changes such as $x+1$, which alter the mathematical field element rather than only its integer representation.

### Targeted repetition

```text
Cases:                                      176
Canonical calls returning status OK:        176
Noncanonical calls returning status OK:     176
Equivalent representatives producing
equal results:                              176

Documentation contract detected:            yes
Contract mismatch classification:           confirmed
```

### Independent native evidence

The successful C probe in the main confirmatory package reproduced:

```text
ARITH with one lane increased by q:           same output
Encoder with one lane increased by 7q:        same output
Branch with all lanes increased by q:         same output
Permutation using the UINT64_MAX residue:     same output
Round-trip of a noncanonical state:           canonical result
```

---

## 6.5 Affected contract

The public surface currently permits two incompatible interpretations.

### Strict interpretation

Every externally supplied state lane must satisfy:

$$
0\leq x<q.
$$

Noncanonical input is invalid.

### Permissive interpretation

Every `uint64_t` is accepted and interpreted modulo $q$.

The API preserves the field element but not the original bit representation.

The current documentation points toward the strict interpretation, while the implementation behaves permissively.

That divergence is the confirmed defect.

---

## 6.6 Security and engineering impact

The internal permutation remains a permutation over:

$$
\mathbb F_q^{16}.
$$

However, the public C function is not injective over the apparent domain of arbitrary `uint64_t[16]` arrays.

For example:

$$
X\neq X'
$$

as arrays of 64-bit integers, while:

$$
X\equiv X' \pmod q
$$

lane by lane, and therefore:

$$
P_\Sigma(X)=P_\Sigma(X')
$$

under the current permissive interpretation.

Potential consequences include:

- byte-level round-trip expectations are violated;
- multiple serialized integer arrays can represent one field state;
- callers may accidentally accept malleable encodings;
- strict and permissive independent implementations can disagree on validity;
- fuzzers may interpret silent canonicalization as unexpected state collapse;
- protocol integrations may fail to enforce unique state encodings;
- error handling is inconsistent with the documented canonical representation.

The high-level Hash256 message encoding remains canonical and was not shown to be directly affected.

---

## 6.7 What was not demonstrated

`F-API-002` does not demonstrate:

- non-injectivity of $P_\Sigma$ over $\mathbb F_q^{16}$;
- a Hash256 collision between two valid message encodings;
- an XOF collision;
- failure of the forward/inverse permutation over canonical states;
- a mathematical defect in modular reduction;
- a practical cryptanalytic attack.

---

## 6.8 Correction option A — strict public boundaries

Under a strict contract, every public cryptographic entry point validates all state lanes before processing.

A reusable validation rule is:

```text
lane >= q → CTC_STATUS_OUT_OF_RANGE
```

Candidate public boundaries include:

- `ctc_sbox_apply`;
- `ctc_sbox_inverse`;
- `ctc_arith_apply`;
- `ctc_arith_inverse`;
- `ctc_arith_apply_encoder`;
- `ctc_arith_inverse_encoder`;
- `ctc_encoder_generate_block`;
- `ctc_encoder_generate_factors`;
- `ctc_branch_apply`;
- `ctc_permutation_apply`;
- `ctc_permutation_inverse`;
- public normalizer and FOLD entry points that accept field-valued objects.

### Implementation guidance

Validation should occur once at the external boundary rather than repeatedly inside every round.

A maintainable design is:

```text
checked public wrapper
        ↓
validated canonical state
        ↓
internal unchecked implementation
```

On validation failure:

- return `CTC_STATUS_OUT_OF_RANGE`;
- leave all caller-provided mutable buffers unchanged;
- do not partially execute the operation.

Elementary field functions may remain permissive because their mathematical purpose is reduction in $\mathbb F_q$.

### Advantages

- matches the current canonical-representation documentation;
- establishes one representation per field element at API boundaries;
- avoids silent input rewriting;
- preserves all results for valid canonical inputs;
- preserves current KATs.

### Disadvantages

- changes behavior for callers that relied on silent reduction;
- requires validation coverage across the full public low-level API.

### Recommended use

This is the recommended correction for the reference implementation.

---

## 6.9 Correction option B — explicitly permissive contract

The implementation may instead document that every `uint64_t` input is reduced modulo $q$.

The public contract would need to state that:

- noncanonical values are accepted;
- canonicalization is silent;
- inverse round-trips preserve the field element, not the input bits;
- distinct arrays may represent the same state;
- callers requiring unique serialization must canonicalize before calling.

### Advantages

- preserves existing behavior;
- requires little or no implementation change.

### Disadvantages

- permits multiple encodings of one state;
- weakens API clarity;
- complicates interoperability;
- is less suitable for a cryptographic reference implementation;
- conflicts with the current documentation and many callers' likely expectations.

### Recommended use

Not recommended unless permissive reduction is an explicit design requirement.

---

## 6.10 Required regression tests

A strict correction must verify:

1. every canonical boundary value succeeds, including $0$ and $q-1$;
2. $q$, $q+1$, `UINT64_MAX`, and representative $x+kq$ values are rejected;
3. all rejected mutable states remain byte-for-byte unchanged;
4. forward and inverse APIs return the same error class;
5. factor-generation output buffers remain unchanged or are deterministically cleared on failure;
6. canonical permutation round-trips remain exact;
7. Hash256, XOF, and canonical permutation KATs remain unchanged;
8. the C and Python test bindings agree on the rejection contract.

---

## 6.11 Disposition

```text
Finding ID:       F-API-002
Behavior:         CONFIRMED
Contract mismatch: CONFIRMED
Mathematical flaw: NOT DEMONSTRATED
Recommended model: strict canonical public inputs
Correction:       required before stable v0.2 release
```

---

# 7. F-SPEC-001 — Capacity and the 256-bit preimage goals

## 7.1 Finding statement

The current documentation lists the following design goals for CTC-Σ-256:

```text
Collision resistance:       up to 2^128 generic operations
Preimage resistance:        up to 2^256 generic operations
Second-preimage resistance: up to 2^256 generic operations
```

The implementation uses:

```text
Field:          F_q, q = 2^61 - 1
State:          16 field lanes
Rate lanes:      8
Capacity lanes:  8
Serialized rate: 40 bytes = 320 bits
Digest:          32 bytes = 256 bits
```

Under the conventional lane-partition model for a permutation-based sponge, the declared capacity is approximately:

$$
c_{\mathrm{lane}}
=
8\log_2(q)
\approx
488\text{ bits}.
$$

Therefore:

$$
\frac{c_{\mathrm{lane}}}{2}
\approx
244\text{ bits}.
$$

This is twelve exponent bits below the documented 256-bit preimage and second-preimage goals.

---

## 7.2 Standard permutation-sponge interpretation

For a conventional truncated sponge built from a random permutation, the generic analysis relates preimage and second-preimage behavior to the digest length $n$ and capacity $c$.

A standard sufficient condition for random-oracle-like preimage resistance is:

$$
n<\frac{c}{2}.
$$

For second-preimage resistance comparable to an $n$-bit random oracle, the conservative condition is likewise associated with a capacity exceeding approximately:

$$
2n.
$$

For CTC-Σ-256 under the declared lane partition:

$$
n=256
$$

and:

$$
\frac{c}{2}\approx244.
$$

Thus:

$$
256<244
$$

is false.

Equivalently, a 256-bit flat claim would conventionally require at least:

$$
c>512\text{ bits}
$$

or a construction-specific argument that justifies a different bound.

The current eight-capacity-lane configuration provides approximately 488 bits, creating:

```text
Capacity shortfall relative to 512 bits:  24 bits
Flat-claim exponent shortfall:            12 bits
```

---

## 7.3 Evidence from the audit suites

### Quick audit

```text
Approximate capacity:             488 bits
Conventional c/2 value:           244 bits
Documented preimage goal:         256 bits
Difference:                        12 bits
```

The quick audit correctly classified this as a parameter-consistency review rather than a practical break.

### Main confirmatory suite

The second suite independently extracted:

```text
q:                   2^61 - 1
state lanes:         16
rate lanes:           8
capacity lanes:       8
serialized rate:     40 bytes
digest:              256 bits
```

Its claim matrix recorded:

| Property | Documented goal | Conventional lane-capacity result |
|---|---:|---:|
| Collision | 128 bits | compatible |
| Preimage | 256 bits | not justified by $c/2\approx244$ |
| Second preimage | 256 bits | not justified by $c/2\approx244$ |

### Targeted confirmation

The targeted suite confirmed the same parameter arithmetic but deliberately tested a second interpretation based on the restricted serialized interface.

Its final classification was:

```text
MODEL_DEPENDENT_NOT_UNCONDITIONALLY_CONFIRMED
```

That refinement is important and is preserved below.

---

## 7.4 Restricted serialized input/output observation

The implementation does not serialize all approximately 61 bits of each rate lane.

During absorption, each rate lane receives a 40-bit little-endian word.

During squeezing, the implementation emits 40 low bits per accepted rate lane.

Therefore:

$$
r_{\mathrm{serialized}}
=
8\cdot40
=
320\text{ bits}.
$$

The complete field-state cardinality is approximately:

$$
b
=
16\log_2(q)
\approx
976\text{ bits}.
$$

A simple cardinality subtraction gives:

$$
b-r_{\mathrm{serialized}}
\approx
656\text{ bits}.
$$

The targeted suite also observed that approximately 21 high bits per first-half lane are not directly emitted:

$$
8\cdot(61-40)
\approx
168\text{ bits}.
$$

Combining those high portions with the last eight lanes gives the same approximate hidden-state cardinality:

$$
488+168\approx656\text{ bits}.
$$

A purely arithmetic $c/2$-style calculation over 656 bits would yield:

$$
656/2\approx328\text{ bits}.
$$

---

## 7.5 Why 656 hidden bits are not automatically 656 capacity bits

The 656-bit number is a state-cardinality observation, not a proven sponge capacity.

In the conventional sponge definition, capacity is a designated state portion that:

- is not directly modified by message injection;
- is not directly returned as output.

The high portions of the first eight CTC-Σ lanes are not directly serialized, but they remain part of the same field coordinates into which message words are added.

Because field addition can propagate through the complete residue, those high portions cannot be reclassified as conventional capacity solely because they are not directly serialized.

Likewise, the rejection-based squeeze map and 40-bit truncation differ from a standard full-rate output map.

Consequently:

```text
approximately 656 hidden cardinality bits
```

does not by itself imply:

```text
approximately 656 proven capacity bits
```

or:

```text
approximately 328 proven security bits.
```

A generalized proof would need to model:

- absorption by addition in $\mathbb F_q$;
- the restricted 40-bit input alphabet;
- the restricted and rejection-conditioned output alphabet;
- access to the public forward and inverse permutation;
- multi-block absorption;
- multi-block squeezing;
- state-binding and path-finding attacks;
- whether the unexposed high portions of the first eight lanes provide independent security comparable to conventional capacity.

---

## 7.6 Correct interpretation

The following statement is confirmed:

> Under the declared eight-rate-lane/eight-capacity-lane permutation-sponge model, the conventional capacity argument does not justify a 256-bit preimage or second-preimage design goal.

The following statements are not confirmed:

```text
The effective preimage security is exactly 244 bits.
A practical 2^244 attack has been constructed.
The high portions of the rate lanes increase proven capacity to 656 bits.
The effective preimage security is approximately 328 bits.
The complete CTC-Σ construction is insecure.
```

No experiment lasting seconds, minutes, or hours can establish or refute a work factor near $2^{256}$.

The third finding is therefore a theoretical claim and modeling issue, not an experimentally demonstrated attack.

---

## 7.7 Collision goal

The 256-bit digest has an ideal generic collision target of:

$$
2^{256/2}=2^{128}.
$$

The conventional lane-capacity exponent is approximately 244 bits, which is above 128 bits.

Therefore, the current parameter review does not create the same immediate claim mismatch for the 128-bit collision goal.

This does not prove collision resistance. It only means that the declared capacity is not the limiting parameter for the nominal 128-bit collision target under the same conservative model.

---

## 7.8 Correction option A — lower the documented preimage goals

The least invasive response is to retain the current state and rate partition and revise the security-goal language.

A conservative interim formulation is:

> Under the conventional lane-partition permutation-sponge model, the eight capacity lanes provide approximately 488 capacity bits and a flat-claim exponent near 244 bits. CTC-Σ does not currently provide a construction-specific proof that the restricted 40-bit absorption and squeeze maps justify a higher bound. Preimage and second-preimage resistance remain unproven design goals.

A more explicit goal table could state:

```text
Collision resistance:
up to 2^128 generic operations — design goal, not proven

Preimage resistance:
conservatively no more than approximately 2^244 under the
conventional lane-partition model — design goal, not proven

Second-preimage resistance:
conservatively no more than approximately 2^244 under the
conventional lane-partition model — design goal, not proven
```

### Advantages

- no implementation change;
- no KAT change;
- removes an unsupported claim;
- preserves current performance and architecture.

### Disadvantages

- reduces the documented preimage target;
- does not resolve whether the restricted interface permits a stronger formal bound.

---

## 7.9 Correction option B — use seven rate lanes and nine capacity lanes

Keeping the 16-lane state while changing the sponge partition to:

```text
Rate:      7 lanes
Capacity:  9 lanes
```

would provide approximately:

$$
c
=
9\log_2(q)
\approx
549\text{ bits}.
$$

Thus:

$$
c/2
\approx
274.5\text{ bits}.
$$

The serialized rate would become:

$$
7\cdot40=280\text{ bits}=35\text{ bytes}.
$$

### Advantages

- exceeds the conventional 512-bit capacity requirement with margin;
- preserves the 16-lane internal permutation;
- preserves the balanced 8+8 Feistel permutation structure;
- directly supports a conventional 256-bit flat-claim target more comfortably.

### Disadvantages

- changes message block size and padding behavior;
- changes absorption and squeezing;
- changes Hash256 and XOF outputs;
- changes all sponge KATs;
- requires specification, API, test, and documentation updates;
- may reduce throughput.

The internal permutation KATs may remain unchanged if the permutation itself is untouched, but all outer-mode vectors must change.

---

## 7.10 Correction option C — enlarge the internal state

Keeping eight serialized rate lanes while increasing the number of capacity lanes requires a larger state.

A simple 17-lane state would permit nine capacity lanes, but it would break the current balanced two-half architecture.

A larger balanced state, such as 18 lanes, would require a deeper redesign of:

- Feistel halves;
- branch width;
- MDS dimensions;
- encoder lane mapping;
- braid integration;
- FOLD dimensions;
- permutation vectors;
- implementation interfaces.

### Advantages

- can preserve a 40-byte rate while increasing conventional capacity;
- may provide larger design margins.

### Disadvantages

- constitutes a major algorithm redesign;
- invalidates nearly every core KAT;
- requires new cryptanalysis;
- changes the mathematical architecture substantially.

This is not a small v0.2 correction.

---

## 7.11 Correction option D — provide a construction-specific proof

The parameters may remain unchanged if a rigorous analysis proves that the restricted input and output maps support the desired claim despite the conventional eight-lane capacity.

Such an analysis must not rely only on:

```text
total state bits - serialized rate bits
```

It must formally model the real interface and generic attacks.

### Advantages

- may preserve the current algorithm and KATs;
- could produce a more accurate security statement.

### Disadvantages

- is mathematically difficult;
- cannot be replaced by empirical avalanche or toy experiments;
- may conclude that the current parameters still need modification;
- should be independently reviewed.

Until such a proof exists, the 256-bit preimage and second-preimage goals should remain explicitly unsubstantiated.

---

## 7.12 Recommended interim documentation language

The following text is suitable while the design decision remains open:

> CTC-Σ-256 outputs 256-bit digests and targets 128-bit collision resistance. Under the conventional permutation-sponge interpretation in which the first eight field lanes form the rate and the final eight lanes form the capacity, the capacity is approximately 488 bits and the conservative flat-claim exponent is approximately 244 bits. The implementation uses restricted 40-bit absorption and squeeze mappings, but no construction-specific proof currently establishes that the unexposed portions of the rate lanes increase the formal capacity bound. Therefore, 256-bit preimage and second-preimage strength are not presently claimed as established properties.

---

## 7.13 Required follow-up analysis

Before selecting a final correction, the project should produce a written model answering:

1. What is the exact state space?
2. What is the exact message-block alphabet?
3. What is the exact output-block alphabet after rejection?
4. Which state coordinates are directly controllable by an adversary?
5. Which state information is directly observable?
6. Does the public inverse permutation alter the generic state-binding attack?
7. What is the generic complexity of binding a 256-bit digest to a valid accepted output state?
8. What is the generic complexity of finding an absorption path to that state?
9. How do message-length encoding and padding affect second-preimage analysis?
10. Which quantity is the justified claimed capacity?

The answer must distinguish a formal proof from a heuristic cardinality argument.

---

## 7.14 Disposition

```text
Finding ID:       F-SPEC-001
Parameter arithmetic: CONFIRMED
244-bit standard-model result: CONFIRMED UNDER THE LANE-PARTITION MODEL
Exact effective security: NOT CONFIRMED
328-bit alternative claim: NOT CONFIRMED
Practical attack: NOT DEMONSTRATED
Nature: theoretical claim/modeling issue
Resolution: specification decision required before stable release
```

---

# 8. Additional observations from the quick audit

## 8.1 O-ENC-001 — v0.2 encoder relation screening

The quick audit tested whether the structural v0.1 input/counter identity reappeared after the v0.2 encoder redesign.

Results:

```text
Old v0.1 shift identity:
0 exact matches in 144 cases

Adjacent-block shifts across all lanes:
0 exact matches in 192 cases

Constant additive relation between adjacent blocks:
0 confirmed relations in 16 tested block pairs
```

Exploratory correlations were:

```text
Maximum absolute same-input block correlation: 0.383810
Maximum absolute independent-control correlation: 0.317313
Samples per round: 24
```

These correlations were based on a small, uncorrected screening sample.

### Disposition

```text
Old exact relation: NOT OBSERVED
Simple additive relation: NOT OBSERVED
Correlation finding: NOT CONFIRMED
Encoder security: NOT ESTABLISHED
```

This evidence supports the narrow statement that the specific tested v0.1 identity did not reappear. It does not establish the absence of all tweak relations.

---

## 8.2 O-SBOX-001 — exact local central symmetry

The isolated S-box test reproduced the expected central symmetry in:

```text
144 of 144 cases
```

The relevant form is:

$$
S(-2C-x)=2A-S(x).
$$

The full configured ARITH compositions were then tested for a corresponding constant-sum relation.

Results:

```text
Configured full-ARITH relations tested: 12
Relations preserved:                    0
```

### Disposition

```text
Local algebraic symmetry: CONFIRMED
Simple tested full-ARITH propagation: NOT OBSERVED
Practical attack: NOT DEMONSTRATED
```

The symmetry remains cryptanalytically relevant and should stay in the research record, but this audit did not establish downstream exploitation.

---

## 8.3 O-BRAID-001 — braid-layer knockout screening

The quick audit compared the complete branch/permutation with a test variant that removed the semantic contribution of braid normalization.

### Branch

```text
Samples:                              32
Full mean Hamming fraction:           0.504995
Knockout mean Hamming fraction:       0.502497
Difference:                           0.002497
Mean changed lanes:                   8 in both variants
```

### Complete permutation

```text
Samples:                               3
Full mean Hamming fraction:            0.494194
Knockout mean Hamming fraction:        0.502732
Difference:                           -0.008538
```

The outputs of the two variants were different, proving that the layer changes the function.

The similar avalanche means do not prove that the braid layer is unnecessary. Avalanche measures only one aspect of diffusion and does not quantify:

- differential probability;
- algebraic complexity;
- invariant destruction;
- integral behavior;
- collision structure;
- preimage resistance;
- resistance to structural distinguishers.

### Disposition

```text
Function changes when braid semantics are removed: CONFIRMED
Security contribution measured by this test: NOT CONFIRMED
Layer redundancy: NOT DEMONSTRATED
Result: INCONCLUSIVE SCREENING
```

A larger knockout study must use attack-oriented metrics, not only mean avalanche.

---

# 9. Test-harness and evidence limitations

## 9.1 Targeted native probe compilation failure

The targeted confirmation suite attempted to compile an additional native C probe using:

```text
-std=c11 -O2 -Wall -Wextra -Werror
```

Compilation stopped because the conditional operator mixed enum and integer signedness:

```text
error: operand of ‘?:’ changes signedness
```

The failing expression was in the test probe, not in CTC-Σ production code.

Therefore:

```text
Targeted C probe compiled: false
Targeted C probe executed: false
Classification: BLOCKED
```

This does not invalidate the Python results, which called the real shared C library through `ctypes`.

It does mean that the targeted package did not provide the additional independent native repetition it intended to provide.

The earlier main confirmatory package did successfully compile and execute its independent C probe, so native confirmation exists for the two API findings.

---

## 9.2 Toy birthday experiment limitation

The targeted suite included a small experiment illustrating approximately $2^{c/2}$ birthday scaling in toy spaces.

That experiment does not measure:

- CTC-Σ preimage resistance;
- CTC-Σ second-preimage resistance;
- the effective capacity of CTC-Σ;
- the security of the production permutation;
- the applicability of the 488-bit or 656-bit models.

It is educational evidence only and must not be cited as confirmation of `F-SPEC-001`.

---

## 9.3 Single-platform limitation

All three audit runs used one principal Linux/x86-64 environment.

Future confirmation should include at least:

- GCC and Clang;
- another x86-64 host;
- one ARM64 environment;
- Debug and Release builds;
- successful ASan and UBSan runs.

The deterministic identities in `F-API-001` and `F-API-002` are source-level and are not expected to depend on architecture, but independent environments improve evidence quality.

---

## 9.4 No high-complexity security conclusion

The audit durations were seconds, not cryptanalytic work factors.

The suites can confirm:

- exact equalities;
- API status behavior;
- source/contract mismatches;
- parameter arithmetic;
- presence or absence of simple relations in sampled cases.

They cannot confirm:

- $2^{244}$ security;
- $2^{256}$ security;
- collision resistance;
- complete differential resistance;
- algebraic resistance;
- production suitability.

---

# 10. Consolidated correction decision matrix

| Finding | Minimal correction | Clean redesign | KAT effect | Recommended immediate action |
|---|---|---|---|---|
| `F-API-001` | Reject legacy indices outside the lossless 16-bit range | Encode all indices independently without truncation | None for validation; complete change for redesign | Add strict range validation |
| `F-API-002` | Reject noncanonical lanes at public boundaries | Redesign public state types around validated canonical objects | None for canonical inputs | Adopt strict canonical contract |
| `F-SPEC-001` | Lower or qualify the preimage goals | Increase capacity or formally redesign/prove the outer mode | None for documentation-only; complete outer-mode change for parameter redesign | Freeze the 256-bit claim pending decision |
| `O-ENC-001` | Preserve regression tests for the removed v0.1 identity | Continue tweak cryptanalysis | None | Keep as regression, not a closed security claim |
| `O-SBOX-001` | Document exact symmetry | Change S-box family if later propagation becomes exploitable | Potentially complete algorithm change | Continue targeted cryptanalysis |
| `O-BRAID-001` | No immediate change | Redesign or remove only after attack-oriented comparison | Complete if changed | Do not modify from avalanche evidence alone |

---

# 11. Recommended correction sequence

## 11.1 Freeze release promotion

Do not promote `0.2.0-dev` to a stable `0.2.0` while:

- `F-API-001` remains accepted by the public API;
- `F-API-002` remains inconsistent with the documented representation contract;
- the preimage-strength statement remains unresolved.

This is a release-quality recommendation, not a statement that the current code is cryptographically broken.

---

## 11.2 Correct the API findings first

The lowest-risk path is:

1. add centralized canonical-state validators;
2. make public cryptographic boundaries reject noncanonical lanes;
3. add legacy index range checks before packed derivation;
4. guarantee no buffer mutation on rejected input;
5. add direct C and Python regression tests;
6. verify that all canonical KATs remain unchanged.

These corrections can be implemented without changing the normative outputs for valid inputs.

---

## 11.3 Decide the sponge claim before freezing final vectors

The project must choose one of the following before final release:

### Path A — retain parameters and lower/qualify claims

Use conservative documentation and preserve the current outer mode.

### Path B — change to seven rate lanes and nine capacity lanes

Accept incompatible Hash256/XOF outputs and regenerate all outer-mode vectors.

### Path C — retain parameters and commission a construction-specific proof

Do not retain an unqualified 256-bit preimage claim while the proof is absent.

### Path D — perform a larger state redesign

Treat as a future major experimental version rather than a local v0.2 correction.

---

## 11.4 Rerun the complete validation matrix

After any correction:

```text
unit tests
integration tests
C/Python cross-implementation tests
forward/inverse tests
Garside normal-form tests
Hash256 and XOF KATs
encoder-block KATs
factor-stream KATs
strict invalid-input tests
legacy alias regression
canonical-state regression
ASan
UBSan
GCC
Clang
```

If the sponge partition changes, repeat all statistical and structural studies that depend on message absorption or output squeezing.

---

# 12. Required post-correction regression criteria

## 12.1 F-API-001 closure criteria

The finding may be closed when:

```text
out-of-range public indices are rejected;
all valid normative indices remain deterministic;
no rejected call mutates state;
legacy KATs remain stable under the validation-only fix;
C and Python tests agree;
the public headers document the exact range.
```

A derivation redesign requires new KATs and a new compatibility statement.

---

## 12.2 F-API-002 closure criteria

The finding may be closed under a strict contract when:

```text
all public cryptographic state APIs reject lane >= q;
all canonical states still succeed;
round-trips remain exact for canonical states;
rejected mutable buffers remain unchanged;
headers and README state the same contract;
cross-implementation tests enforce rejection.
```

Under a permissive contract, closure instead requires explicit documentation of canonicalization and a deliberate acceptance of multiple encodings.

---

## 12.3 F-SPEC-001 closure criteria

The finding may be closed only by one of the following:

1. the security-goal table is changed to match the conservative conventional model;
2. the capacity is increased and all affected evidence is regenerated;
3. a reviewed construction-specific proof justifies the retained 256-bit goals.

A toy experiment, avalanche test, or absence of practical preimages cannot close this finding.

---

# 13. Statements that must not be made from this evidence

The audit does not support the following statements:

```text
CTC-Σ has been broken.
CTC-Σ has exactly 244-bit preimage security.
CTC-Σ has 328-bit preimage security.
The braid layer is useless.
The S-box symmetry is exploitable.
The v0.2 encoder has no remaining structural relations.
The API findings create a valid-message Hash256 collision.
Passing the correction regressions proves cryptographic security.
```

The defensible statements are:

```text
Two low-level public-API contract defects were confirmed.

The specific v0.1 encoder input/counter identity did not reappear in
the tested v0.2 relation screens.

The conventional eight-capacity-lane model does not justify an
unqualified 256-bit preimage or second-preimage goal.

The exact effective generic security of the restricted F_q sponge
interface remains unproven.

No practical full-construction attack was demonstrated.
```

---

# 14. Proposed issue and changelog language

## 14.1 F-API-001 issue summary

```text
Legacy generic ARITH index aliasing

The legacy constant provider truncates round and subround identifiers
to 16 bits before packing. Public identifiers separated by multiples
of 2^16 therefore select identical constants while returning success.
The normative v0.2 path uses small indices, so no Hash256/XOF collision
has been demonstrated. The public API must either reject out-of-range
indices or adopt a lossless constant-domain encoding.
```

## 14.2 F-API-002 issue summary

```text
Noncanonical public-state acceptance

Low-level public APIs accept uint64_t lane values greater than or equal
to q, silently reduce them modulo q, and return canonical outputs. This
is mathematically valid over F_q but conflicts with the documented
canonical external representation. The project must adopt either strict
boundary validation or an explicitly permissive canonicalization
contract. Strict validation is recommended.
```

## 14.3 F-SPEC-001 issue summary

```text
Preimage-goal justification under the sponge model

With eight capacity lanes over F_(2^61-1), the conventional lane-based
capacity is approximately 488 bits and the corresponding flat-claim
exponent is approximately 244 bits. This does not support an unqualified
256-bit preimage or second-preimage goal under the standard
permutation-sponge model. The restricted 40-bit input/output mapping may
change the exact generic analysis, but no construction-specific proof is
currently available. The project must qualify the claim, increase
capacity, or provide a reviewed proof.
```

---

# 15. Final assessment

The v0.2 encoder redesign successfully removed the previously confirmed direct input/block-counter identity from the tested relation families.

The subsequent audits nevertheless identified two genuine low-level interface defects:

```text
F-API-001:
distinct public indices alias because of 16-bit truncation;

F-API-002:
noncanonical public states are silently accepted despite a canonical
representation contract.
```

Both findings were reproduced across multiple suites, appropriate controls, high-volume cases, and at least one successful independent C probe.

They should be corrected before a stable release.

The capacity review produced a different type of result:

```text
F-SPEC-001:
the current 256-bit preimage and second-preimage goals are not justified
by the conventional declared-capacity model.
```

This does not demonstrate an actual $2^{244}$ attack and does not prove that the restricted serialized interface provides a stronger bound. It establishes a specification obligation: the project must either reduce the claim, increase the capacity, or supply a construction-specific proof.

The other observations remain research items:

- the v0.1 encoder identity was not observed after the v0.2 redesign;
- local S-box symmetry is exact but was not shown to propagate through the tested full ARITH relation;
- the braid-knockout avalanche screen was too small and too narrow to justify a design change.

The appropriate release posture is:

```text
Implementation/API correction: REQUIRED
Security-claim clarification:   REQUIRED
Practical break:                NOT DEMONSTRATED
Production use:                 NOT AUTHORIZED
```

---

# Appendix A — Consolidated numerical evidence

## A.1 F-API-001

| Evidence group | Cases | Confirming outcomes |
|---|---:|---:|
| Quick ARITH aliases | 72 | 72 |
| Quick S-box aliases | 192 | 192 |
| Confirmatory ARITH aliases | 3,000 | 3,000 |
| Confirmatory S-box aliases | 21,600 | 21,600 |
| Direct constant aliases | 9,600 | 9,600 |
| Collapsed nominal domain groups | 15 | 15 |
| Targeted alias repetition | 900 | 900 |
| Confirmatory neighbor controls | 4,920 | 4,920 different |
| Targeted neighbor controls | 900 | 900 different |

## A.2 F-API-002

| Evidence group | Cases | Equivalent outputs |
|---|---:|---:|
| Quick ARITH | 12 | 12 |
| Quick permutation | 3 | 3 |
| Confirmatory scalar S-box | 4,176 | 4,176 |
| Confirmatory vector APIs | 772 | 772 |
| Confirmatory factor streams | 30 | 30 |
| Confirmatory round-trips | 30 | 30 canonicalized |
| Genuine-change controls | 24 | 24 different |
| Targeted public-state repetition | 176 | 176 |

## A.3 F-SPEC-001

```text
Field modulus:                   2^61 - 1
Approximate field-lane bits:     61
State lanes:                     16
Rate lanes:                       8
Capacity lanes:                   8
Serialized rate bytes:           40
Serialized rate bits:           320
Digest bits:                    256

Lane-partition capacity:        approximately 488 bits
Lane-partition c/2:             approximately 244 bits
Capacity needed for c/2 > 256:  greater than 512 bits

Nine-lane capacity:             approximately 549 bits
Nine-lane c/2:                  approximately 274.5 bits
```

---

# Appendix B — Relevant implementation locations

```text
include/ctc_sigma/parameters.h
    field modulus, state width, rate lanes, rate bytes, digest bytes

include/ctc_sigma/arith.h
    public generic ARITH and S-box interfaces

src/arith.c
    legacy primary/secondary index packing
    generic and encoder constant providers
    ARITH forward and inverse paths

src/field.c
    uint64_t reduction modulo q

include/ctc_sigma/encoder.h
src/encoder.c
    v0.2 raw encoder blocks and factor generation

include/ctc_sigma/branch.h
src/branch.c
    branch public boundary and A_PRE/A_POST use

include/ctc_sigma/permutation.h
src/permutation.c
    public permutation forward and inverse boundaries

include/ctc_sigma/sponge.h
src/sponge.c
    40-byte absorption and squeeze interface
    rejection-conditioned 40-bit output mapping

README.md
    canonical field-element representation
    current security-goal table

CHANGELOG.md
    v0.2 compatibility and correction history
```

---

# Appendix C — Reference basis for the theoretical review

The theoretical interpretation in `F-SPEC-001` is based on the conventional analysis of truncated sponge functions using a random permutation, particularly the treatment of preimage and second-preimage resistance in:

```text
Guido Bertoni, Joan Daemen, Michaël Peeters, and Gilles Van Assche,
Cryptographic Sponge Functions, Version 0.1, 2011,
Sections 5.10.2, 5.10.3, 6.6, and 7.2.
```

The reference applies to the conventional sponge model. CTC-Σ uses field addition, a restricted 40-bit block alphabet, and rejection-conditioned output, so a direct stronger claim requires a construction-specific proof rather than a cardinality analogy.

---

# Maintainer note

This document should remain readable by:

- implementers;
- cryptographers;
- API reviewers;
- test engineers;
- maintainers deciding compatibility policy;
- external reviewers assessing project maturity.

Technical precision takes priority over promotional language.

Confirmed defects, model-dependent conclusions, blocked tests, and inconclusive observations must remain clearly separated.
