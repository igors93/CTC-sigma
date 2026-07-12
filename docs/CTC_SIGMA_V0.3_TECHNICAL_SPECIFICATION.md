---
title: "CTC-Sigma v0.3 Technical Specification"
subtitle: "Experimental architecture - version 0.3.0-dev"
date: "2026-07-12"
lang: en
geometry: margin=25mm
fontsize: 10pt
header-includes:
  - |
    \usepackage{longtable}
  - |
    \usepackage{booktabs}
  - |
    \usepackage{microtype}
---

# 1. Status, scope, and historical policy

CTC-Sigma v0.3 is an **experimental and incompatible architectural revision** of the CTC-Sigma hash and extendable-output construction. It is a research artifact, not a production cryptographic standard.

This document specifies the behavior implemented by the `0.3.0-dev` source tree. It does not retroactively replace the v0.1 or v0.2 records. Earlier specifications, known-answer vectors, audit findings, and research reports remain preserved as historical evidence. In particular:

- v0.1 records the first complete construction;
- v0.2 records the tweakable encoder redesign and its validation;
- v0.3 changes the branch architecture, constant domain, round schedule, sponge partition, factor count, and all normative outputs.

The change from v0.2 to v0.3 is not described as proof of a practical break in v0.2. It is a design response to confirmed implementation-contract issues, a capacity-model concern, and an unresolved question about the measurable role of the braid layer.

> **Security warning.** No collision, preimage, second-preimage, pseudorandomness, indifferentiability, or post-quantum security property has been proven. CTC-Sigma v0.3 must not be used to protect real data.

# 2. Normative notation

All arithmetic is performed in the prime field

$$
\mathbb{F}_q, \qquad q = 2^{61}-1.
$$

A field lane is represented canonically by an unsigned integer in the interval

$$
0 \le x < q.
$$

The state contains sixteen lanes:

$$
S = (s_0,\ldots,s_{15}) \in \mathbb{F}_q^{16}.
$$

The permutation is a balanced Feistel network with halves

$$
L,R \in \mathbb{F}_q^8.
$$

Indices of eight-lane vectors are interpreted modulo 8 when explicitly stated. Byte encodings are little-endian. `LE32`, `LE40`, and `LE64` denote fixed-width little-endian encodings. `IntegerLE` interprets a byte string as a non-negative little-endian integer.

`SHAKE256(X,n)` denotes the first `n` bytes of SHAKE256 applied to `X`.

# 3. Parameter set

| Parameter | v0.3 value | Meaning |
|---|---:|---|
| Version string | `0.3.0-dev` | Experimental implementation version |
| Field modulus | $q=2^{61}-1$ | Prime field |
| State width | 16 lanes | Approximately 976 field-state bits |
| Feistel halves | 8 + 8 lanes | Left and right halves |
| Feistel rounds | 12 | Total permutation rounds |
| Braid-active rounds | 8 | Rounds using encoding, normalization, descriptor, and dynamic arithmetic |
| Arithmetic-bridge rounds | 4 | Rounds using fixed arithmetic only |
| Schedule | `B B A` repeated four times | `B` = braid-active, `A` = bridge |
| Factors per braid-active round | 24 | Accepted signed simple factors |
| Braid group | $B_8$ | Eight strands |
| Retained normal-form window | 16 factors | Last factors tagged as `KEEP` |
| `A_PRE` subrounds | 3 | Fixed arithmetic pre-mixing |
| `A_ENC` subrounds | 4 | Tweakable encoder block permutation |
| Dynamic arithmetic subrounds | 4 | Descriptor-controlled post-transformation |
| Bridge subrounds | 3 | Fixed arithmetic bridge |
| Rate | 7 lanes / 35 bytes | Seven serialized 40-bit words |
| Capacity | 9 lanes | Conventional lane partition of about 549 bits |
| Hash output | 32 bytes | CTC-Sigma-256 |
| XOF output | arbitrary | CTC-Sigma-XOF |

The state cardinality is $q^{16}$. The declared conventional sponge capacity is $q^9$, for which

$$
\log_2(q^9) \approx 549.
$$

This parameter is large enough that the simple numerical quantity $c/2$ exceeds 256 bits, but that observation alone is **not** a proof of 256-bit preimage or second-preimage security.

# 4. Public constant derivation

v0.3 resets the public constant domain. Normative v0.3 constants do not use the v0.1 or v0.2 prefixes.

## 4.1 Scalar derivation

For component string $C$, purpose identifier $P$, round $i$, subround $s$, lane $j$, and block $h$, define

$$
\begin{aligned}
\operatorname{Seed}_{0.3}(C,P,i,s,j,h) ={}&
\texttt{"CTC-SIGMA-v0.3|CONST|"} \\
&\|\ \operatorname{LEN8}(C) \| C \\
&\|\ \operatorname{LE32}(P) \\
&\|\ \operatorname{LE32}(i) \\
&\|\ \operatorname{LE32}(s) \\
&\|\ \operatorname{LE32}(j) \\
&\|\ \operatorname{LE32}(h).
\end{aligned}
$$

The field constant is

$$
\operatorname{Const}_{0.3}(C,P,i,s,j,h)
=
\operatorname{IntegerLE}\!\left(
\operatorname{SHAKE256}(\operatorname{Seed}_{0.3},16)
\right) \bmod q.
$$

The purpose identifiers are fixed by the public enumeration:

| Identifier | Value | Use |
|---|---:|---|
| `GENERIC` | 0 | Compatibility helper and fold domains |
| `ROUND` | 1 | Round-addition constants |
| `SBOX_A` | 2 | S-box additive output constants |
| `SBOX_B` | 3 | S-box multiplicative constants |
| `SBOX_C` | 4 | S-box input translations |
| `DESCRIPTOR_IV` | 5 | Braid descriptor domains |
| `DESCRIPTOR_GROUP` | 6 | Reserved descriptor grouping domain |
| `DYNAMIC_RC` | 7 | Dynamic round constants |
| `DYNAMIC_A` | 8 | Dynamic S-box $A$ constants |
| `DYNAMIC_B` | 9 | Dynamic S-box $B$ constants |
| `DYNAMIC_C` | 10 | Dynamic S-box $C$ constants |
| `SPONGE_IV` | 11 | Sponge initialization vectors |

Whenever an S-box multiplicative constant is zero after reduction, the implementation replaces it with one.

## 4.2 Eight-lane batch derivation

Several v0.3 components derive eight lanes with one SHAKE invocation. The seed is the scalar seed with the lane field set to `0xFFFFFFFF`. SHAKE256 emits 128 bytes, split into eight consecutive 16-byte chunks. Each chunk is independently interpreted little-endian and reduced modulo $q$.

This batch operation is distinct from calling the scalar derivation for lane indices 0 through 7.

# 5. Fixed arithmetic permutation `ARITH`

For a component label $C$, round $i$, and subround number $s$, the standard arithmetic layer derives, for each lane $j$:

- round constant $r_{s,j}$ with purpose `ROUND`;
- S-box constants $a_{s,j}$, $b_{s,j}$, and $c_{s,j}$ with purposes `SBOX_A`, `SBOX_B`, and `SBOX_C`;
- block index zero.

The subround degree is

$$
d_s =
\begin{cases}
23, & s \equiv 0 \pmod 2,\\
47, & s \equiv 1 \pmod 2.
\end{cases}
$$

Let $D_d(x)=D_d(x,1)$ be the Dickson polynomial defined by

$$
D_0(x)=2,\qquad D_1(x)=x,\qquad
D_n(x)=xD_{n-1}(x)-D_{n-2}(x).
$$

Field inversion uses the totalized convention

$$
\operatorname{Inv}(0)=0.
$$

The lane S-box is

$$
Y_{s,j}
=
a_{s,j}
+
b_{s,j}\operatorname{Inv}\!\left(
D_{d_s}(X_{s,j}+r_{s,j}+c_{s,j})
\right).
$$

The diffusion matrix is the 8 by 8 Cauchy matrix

$$
M_{u,v}=(u+v+18)^{-1},
\qquad 0\le u,v<8.
$$

One subround returns

$$
X_{s+1}=M Y_s.
$$

`ARITH(C,i,X,rho)` applies `rho` consecutive subrounds starting with subround index zero. The implementation also provides the inverse for the fixed arithmetic path and tests forward/inverse round trips.

# 6. Tweakable encoder and signed factors

The encoder consumes the pre-mixed state $u\in\mathbb{F}_q^8$. For Feistel round $i$ and encoder block $h$, define

$$
v_h=A_{\mathrm{ENC},i,h}(u),
$$

where $A_{\mathrm{ENC},i,h}$ is a four-subround arithmetic permutation. Its data input is copied unchanged; the block index is used only in public constant derivation.

The component name is `A_ENC`. The four constant families use purposes `ROUND`, `SBOX_A`, `SBOX_B`, and `SBOX_C`, with the complete tuple $(i,s,j,h)$ serialized losslessly.

Each lane candidate $x\in[0,q)$ is decoded as follows. Define

$$
T_{32}=\left\lfloor\frac{q}{2^{32}}\right\rfloor 2^{32}.
$$

If $x\ge T_{32}$, reject the candidate. Otherwise set

$$
w=x\bmod 2^{32},\qquad z=w\bmod 2^{31}.
$$

Define

$$
T_B=
\left\lfloor\frac{2^{31}}{40319}\right\rfloor 40319.
$$

If $z\ge T_B$, reject the candidate. Otherwise produce

$$
\varepsilon=
\begin{cases}
+1,& \text{bit 31 of }w\text{ is zero},\\
-1,& \text{bit 31 of }w\text{ is one},
\end{cases}
$$

and

$$
r=1+(z\bmod 40319).
$$

The result is the signed simple factor $(r,\varepsilon)$. Blocks are generated until exactly 24 factors have been accepted or the defensive block limit is reached.

Ranks are canonical Lehmer ranks of permutations in $S_8$:

- rank 0 is the identity;
- rank 40319 is the Garside element $\Delta$;
- ranks 1 through 40318 are proper simple factors.

# 7. Left Garside normal form

The 24 accepted signed factors define a word $W$ in $B_8$. The normalizer computes the left Garside normal form

$$
\operatorname{NF}(W)=\Delta^p x_1x_2\cdots x_m.
$$

The stored result satisfies:

- $p\in\mathbb{Z}$ is the infimum;
- every stored $x_k$ is a proper simple factor with Lehmer rank 1 through 40318;
- identity factors are omitted;
- $\Delta$ factors are absorbed into $p$;
- every adjacent pair is left-weighted;
- $m\le512$.

Negative simple factors are rewritten using the right complement and the flip automorphism before left-weighting. The C implementation is cross-checked against an independent Python implementation.

An externally injected normalizer is trusted for semantic equivalence to the input word, but its returned representation is validated for canonical structure before it is used.

# 8. Normal-form tokenization and fold

Let

$$
k=\max(0,m-16)
$$

be the prefix length outside the retained 16-factor window. The normal form is serialized into field tokens

$$
\operatorname{Token}(t,v)=t\cdot 2^{48}+v.
$$

The ordered token stream begins with:

1. version token with value 3;
2. Feistel-round token;
3. zigzag-encoded infimum token;
4. canonical-length token $m$;
5. prefix-length token $k$.

Every factor $x_j$ is then emitted in order:

- factors with $j<k$ use the `DROP_FACTOR` tag;
- factors with $j\ge k$ use the `KEEP_FACTOR` tag.

The names `DROP` and `KEEP` identify structural regions; no factor is silently discarded from the token stream.

`FOLD_NF` initializes eight lanes with the `FOLD-IV` domain, absorbs at most four tokens per group into lanes 0 through 3, adds group metadata to lane 4, adds group-domain constants to lanes 5 through 7, and applies two `A_FOLD` subrounds after each group. It then adds an explicit token-count marker and end marker and applies four final `A_FOLD` subrounds.

The result is

$$
h=\operatorname{FOLD}_{i}(\operatorname{NF}(W))
\in\mathbb{F}_q^8.
$$

# 9. Braid descriptor

For a braid-active round $i$, derive the eight-lane descriptor-domain vector

$$
d=\operatorname{ConstLanes}(
\texttt{"BRAID-DESCRIPTOR"},
\texttt{DESCRIPTOR\_IV},
i,0,0
).
$$

For $0\le j<8$, with indices modulo 8, form

$$
z_j
=
h_j+d_j+(j+2)h_{j+3}+h_{j+5}^2.
$$

Apply two fixed arithmetic subrounds:

$$
\delta
=
\operatorname{ARITH}(
\texttt{"BRAID-DESCRIPTOR-FINAL"},
i,z,2
).
$$

The descriptor object stores $\delta$, the token count, and the prefix-factor count. The dynamic arithmetic configuration is determined by the descriptor lanes $\delta$.

# 10. Descriptor injection and dynamic arithmetic

## 10.1 State injection

The pre-mixed state $u$ and descriptor $\delta$ are coupled before the nonlinear dynamic layer:

$$
v_j
=
u_j+\delta_j+(j+2)\delta_{j+3}+\delta_{j+5}^2,
\qquad 0\le j<8.
$$

This direct injection prevents the descriptor from acting only as remote control metadata.

## 10.2 Descriptor-selected lane permutation

Define

$$
T_\pi=
\left\lfloor\frac{q}{40320}\right\rfloor 40320.
$$

Scan $\delta_0,\ldots,\delta_7$ in order. The first lane below $T_\pi$ selects

$$
r_\pi=\delta_j\bmod 40320.
$$

Lehmer unranking maps $r_\pi$ to a permutation

$$
\pi\in S_8.
$$

If all eight lanes fail the rejection condition, the operation returns the defensive `REJECTION_LIMIT` status.

## 10.3 Descriptor-dependent constants and degree order

For dynamic subround $s\in\{0,1,2,3\}$ and lane $j$, define

$$
\mu_{s,j}
=
\delta_j
+(s+2)\delta_{j+s+1}
+(j+3)\delta_{j+2s+3}^{2}.
$$

All descriptor indices are modulo 8. Let $R^0_{s,j}$, $A^0_{s,j}$, $B^0_{s,j}$, and $C^0_{s,j}$ be the corresponding eight-lane batch constants derived under component `DYNAMIC-POST` and purposes `DYNAMIC_RC`, `DYNAMIC_A`, `DYNAMIC_B`, and `DYNAMIC_C`.

The effective constants are

$$
R_{s,j}=R^0_{s,j}+\mu_{s,j},
$$

$$
A_{s,j}=A^0_{s,j}+\delta_{j+s+2},
$$

$$
B_{s,j}=B^0_{s,j}+\mu_{s,j}+1,
$$

$$
C_{s,j}=C^0_{s,j}+\delta_j+\mu_{s,j}.
$$

A zero value of $B_{s,j}$ is replaced by one.

Let

$$
b=(\delta_7+i)\bmod2.
$$

The dynamic degree is

$$
d_s=
\begin{cases}
47,& s\bmod2=b,\\
23,& s\bmod2\ne b.
\end{cases}
$$

Thus every dynamic call uses degree 23 twice and degree 47 twice, while the descriptor determines which degree comes first.

## 10.4 Dynamic subround

For current state $X_s$, first apply

$$
Y_{s,j}
=
A_{s,j}
+
B_{s,j}\operatorname{Inv}\!\left(
D_{d_s}(X_{s,j}+R_{s,j}+C_{s,j})
\right).
$$

The descriptor-dependent diffusion is

$$
X_{s+1,r}
=
\sum_{c=0}^{7}
M_{\pi(r+s),\,\pi(c+s)}Y_{s,c},
$$

where permutation indices are taken modulo 8 before applying $\pi$. Four dynamic subrounds are applied.

# 11. Branch functions and round schedule

The Feistel round schedule is

$$
B,B,A,\ B,B,A,\ B,B,A,\ B,B,A.
$$

## 11.1 Braid-active round `B`

For right half $R_i$, a full braid-active branch computes

$$
u=\operatorname{ARITH}(\texttt{"V03-A-PRE"},i,R_i,3),
$$

$$
W=\operatorname{Encode}_{i}(u),
$$

$$
N=\operatorname{NF}_{B_8}(W),
$$

$$
\delta=\operatorname{BraidDescriptor}_{i}(N),
$$

$$
v=\operatorname{Inject}(u,\delta),
$$

$$
F_i(R_i)=\operatorname{DynamicArith}_{i,\delta}(v).
$$

The architectural principle of v0.3 is therefore:

$$
\boxed{
\text{the normalized braid descriptor changes both the data and the arithmetic transformation applied to it}
}
$$

Different normal forms are expected to select different constants, lane permutations, and degree order. This establishes an architectural dependency, not a proven cryptographic advantage.

## 11.2 Arithmetic bridge round `A`

An arithmetic bridge round does not invoke the encoder or braid normalizer:

$$
F_i(R_i)
=
\operatorname{ARITH}(\texttt{"V03-BRIDGE"},i,R_i,3).
$$

Bridge rounds separate pairs of braid-active rounds and provide a fixed arithmetic path for comparative analysis.

## 11.3 Research-only controlled variants

The public research interface exposes five branch variants for ablation studies:

| Variant | Behavior in a braid-active round |
|---|---|
| `FULL` | Normative v0.3 branch |
| `EMPTY_NORMAL_FORM` | Encoder runs, but the descriptor is built from the canonical empty normal form |
| `FIXED_DESCRIPTOR` | Uses a public fixed descriptor independent of the input braid |
| `NO_DYNAMIC_CONTROL` | Injects the computed descriptor, then uses fixed post-arithmetic |
| `ARITHMETIC_ONLY` | Uses `A_PRE` followed by a fixed arithmetic post-layer |

In arithmetic bridge rounds, all variants intentionally use the same bridge function. These controls are experimental tools and do not define alternative CTC-Sigma versions.

# 12. Feistel permutation

For round $i=0,\ldots,11$, define

$$
T_i=F_i(R_i),
$$

$$
L_{i+1}=R_i,
$$

$$
R_{i+1}=L_i+T_i.
$$

All additions are in $\mathbb{F}_q^8$. The output permutation is

$$
P_\Sigma(L_0,R_0)=(L_{12},R_{12}).
$$

The inverse uses

$$
R_i=L_{i+1},
$$

$$
L_i=R_{i+1}-F_i(L_{i+1}).
$$

The Feistel structure is bijective for every deterministic branch function. This structural fact does not imply cryptographic strength.

# 13. Sponge construction

## 13.1 State partition

The rate is lanes 0 through 6. The capacity is lanes 7 through 15.

Each rate block contains seven 40-bit little-endian words:

$$
r=7\cdot40=280\text{ serialized bits}=35\text{ bytes}.
$$

## 13.2 Domains and initialization

The public domains are:

| Function | Domain byte | IV component label |
|---|---:|---|
| Hash256 | `0x01` | `IV-HASH` |
| XOF | `0x02` | `IV-XOF` |
| Internal test | `0x7F` | `IV-TEST` |

Rate lanes are initialized to zero. Capacity lane $j$ is initialized with the scalar v0.3 constant under purpose `SPONGE_IV`, round zero, subround zero, lane index $j$, and block field equal to the domain byte.

## 13.3 Injective message encoding

For message $M$ and domain byte $d$, the encoded input is

$$
M
\|\operatorname{LE64}(|M|)
\|d
\|\texttt{0x01}
\|0^z
\|\texttt{0x80},
$$

where $z$ is the unique non-negative number of zero bytes that makes the total length a multiple of 35 bytes.

The explicit byte length, function domain, start marker, and final marker make the encoding syntactically decodable.

## 13.4 Absorption

For each encoded block, parse seven words $m_0,\ldots,m_6$ with `LE40` and update

$$
s_j\leftarrow s_j+m_j,
\qquad 0\le j<7.
$$

Then apply $P_\Sigma$ to the complete 16-lane state.

## 13.5 Bias-free squeezing

Define

$$
T_{40}=\left\lfloor\frac{q}{2^{40}}\right\rfloor2^{40}.
$$

A state is accepted for output only when every rate lane satisfies

$$
s_j<T_{40},\qquad 0\le j<7.
$$

If any rate lane fails, apply $P_\Sigma$ and test again. On acceptance, emit the low 40 bits of rate lanes 0 through 6 in little-endian order, truncating only the final requested output block. Apply another permutation when more output is required.

The implementation enforces a defensive maximum number of rejection iterations.

## 13.6 Public functions

CTC-Sigma-256 returns the first 32 bytes squeezed under the Hash256 domain. CTC-Sigma-XOF returns the requested number of bytes under the XOF domain.

# 14. Canonical API behavior

Normative public functions reject non-canonical field inputs rather than reducing them implicitly. Inputs must satisfy $0\le x<q$ for every supplied lane. On validation failure, state/output buffers covered by the tests remain unmodified.

All fallible C functions return `ctc_status_t`. Defensive statuses include invalid argument, out of range, allocation failure, rejection limit, and internal error. The library does not terminate the host process.

# 15. Known-answer vectors and reproducibility

The active v0.3 vectors are:

- `test/vectors/ctc_sigma_v03_kat.json`;
- `test/vectors/ctc_sigma_v03_encoder_constants.json`;
- `test/vectors/ctc_sigma_v03_encoder_constants.sha256`.

The KAT set covers Hash256, XOF, permutation forward/inverse, selected encoder blocks, a complete 24-factor stream, and representative encoder constants. The generator is `scripts/generate_kat.py`.

The following historical files remain preserved and must not be regenerated with v0.3 behavior:

- `ctc_sigma_v01_kat.json`;
- `ctc_sigma_v02_kat.json`;
- `ctc_sigma_v02_encoder_constants.json`;
- `ctc_sigma_v02_encoder_constants.sha256`.

# 16. Security goals and non-claims

| Property | Research target | Current status |
|---|---|---|
| Collision resistance | Up to generic $2^{128}$ work for 256-bit output | Not proven |
| Preimage resistance | Up to generic $2^{256}$ work | Not proven |
| Second-preimage resistance | Up to generic $2^{256}$ work for unstructured messages | Not proven |
| XOF pseudorandomness | No practical distinguisher | Not proven |
| Braid contribution | Detectable and attack-relevant structural contribution | Architecture changed; benefit not established |
| Reduced-round resistance | No shortcut below declared targets | Open research |
| Post-quantum security | No claim | Not evaluated as a standardized PQ primitive |

The following statements are explicitly rejected:

- state width alone proves a security level;
- a braid-group component is secure because braid problems can be hard;
- dynamic constants automatically improve security;
- passing avalanche tests proves cryptographic security;
- the v0.3 redesign invalidates or conceals the v0.2 research record;
- KAT reproducibility is an independent security review.

# 17. Required research before stabilization

Before a stable release or security claim, the project requires at least:

1. an independent implementation of the complete v0.3 constant, descriptor, dynamic arithmetic, permutation, and sponge paths;
2. high-volume differential and rotational studies of braid-active and bridge schedules;
3. attack-oriented ablation comparisons using all research variants;
4. algebraic analysis of the dynamic S-box and descriptor-dependent MDS indexing;
5. reduced-round distinguishers, invariant-subspace searches, and meet-in-the-middle analysis;
6. analysis of descriptor collisions and the effect of normal-form compression;
7. cross-platform vector reproduction and sanitizer/fuzzing campaigns;
8. formal review of the outer sponge mode and its claimed bounds;
9. public cryptanalysis by researchers independent of the implementation authors.

# 18. Compatibility statement

CTC-Sigma v0.3 is intentionally incompatible with v0.2. The following are changed:

- public constant seed prefix and tuple encoding;
- fixed arithmetic constants;
- encoder constants and outputs;
- factors per braid-active branch, from 32 to 24;
- branch architecture and descriptor-controlled arithmetic;
- round schedule, now eight braid-active plus four bridge rounds;
- sponge rate/capacity partition, from 8/8 lanes to 7/9 lanes;
- serialized rate block, from 40 to 35 bytes;
- message-boundary vectors;
- permutation, Hash256, and XOF outputs;
- token version, now 3.

No v0.1 or v0.2 digest, vector, constant manifest, or specification is normative for v0.3.

# Appendix A. Normative component labels

The implementation uses the following labels in normative v0.3 paths:

- `A_ENC`
- `V03-A-PRE`
- `V03-BRIDGE`
- `FOLD-IV`
- `FOLD-GROUP`
- `A_FOLD`
- `BRAID-DESCRIPTOR`
- `BRAID-DESCRIPTOR-FINAL`
- `DYNAMIC-POST`
- `IV-HASH`
- `IV-XOF`
- `IV-TEST`

Research controls additionally use:

- `FIXED-DESCRIPTOR-CONTROL`
- `V03-FIXED-POST-CONTROL`
- `V03-ARITHMETIC-ONLY`

# Appendix B. Source-of-truth mapping

For implementation details, the primary files are:

- parameters: `include/ctc_sigma/parameters.h`;
- constants: `src/constants.c`;
- fixed arithmetic: `src/arith.c`;
- factor encoder: `src/encoder.c`;
- Garside normalizer: `src/braid.c`;
- fold: `src/fold.c`;
- descriptor: `src/braid_descriptor.c`;
- dynamic arithmetic: `src/dynamic_arith.c`;
- branch and schedule: `src/branch.c`, `src/round_schedule.c`;
- permutation: `src/permutation.c`;
- sponge: `src/sponge.c`.

Where prose and implementation disagree during the `0.3.0-dev` phase, the discrepancy must be recorded and resolved rather than silently normalized.
