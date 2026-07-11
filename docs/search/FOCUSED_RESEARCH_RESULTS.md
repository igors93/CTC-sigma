# CTC-Σ v0.2 — Focused Open-Question Study

**Document status:** Completed technical study record  
**Project:** CTC-Σ  
**Implementation version:** `0.2.0-dev`  
**Run identifier:** `20260711T215410Z-focused-open-questions-v02`  
**Execution date:** 11 July 2026  
**Purpose:** Record the design, execution, evidence, results, interpretation, limitations, and correction relevance of the focused study performed to investigate the open encoder, S-box, braid-layer, and sponge-capacity questions that remained after the v0.2 audit.

> **Security notice**
>
> CTC-Σ remains an experimental cryptographic construction. This study confirms several exact reduced-round mathematical relations and excludes several simple relations within the tested scope. It does not prove collision resistance, preimage resistance, second-preimage resistance, differential security, algebraic security, side-channel security, or production suitability.

---

## 1. Scope of this study

This study was designed specifically to investigate four questions that remained unresolved after earlier v0.2 audits:

1. **Encoder relations**  
   Determine whether exact, constant-difference, or affine relations remain between different v0.2 encoder tweaks, and whether any such relation survives all four normative encoder subrounds.

2. **S-box symmetry propagation**  
   Confirm the exact local central symmetry of the S-box and determine whether its induced relation survives one or more complete ARITH subrounds.

3. **Braid-layer contribution**  
   Compare the production branch with a controlled branch variant in which the braid normal form is replaced by an empty valid normal form, using more evidence than the earlier small avalanche screen.

4. **Sponge-capacity interpretation**  
   Use exhaustive small-state models to test whether partially exposed state bits may be treated as ordinary sponge capacity solely by subtracting the serialized input/output width from the full state width.

The study also repeated the previously blocked native C probe using corrected source code and strict compiler warnings.

---

## 2. Result vocabulary

The following terms have precise meanings in this document.

### CONFIRMED

An exact behavior was reproduced for every configured case and is consistent with the implementation or a mathematical derivation.

### NOT OBSERVED

The specified relation was not found in the tested family and sample.

This does not prove that no other relation exists.

### NO MEASURABLE EFFECT IN THE TESTED METRICS

The observed difference did not exceed the predeclared practical threshold and did not show a sufficiently consistent direction across independent groups.

This classification applies only to the measured metrics.

### MODEL EVIDENCE ONLY

The result illustrates behavior in exhaustive reduced models but does not establish a security claim for the production construction.

### INCONCLUSIVE FOR GENERAL SECURITY

The experiment does not justify a conclusion about the complete cryptographic value, redundancy, or security of a component.

---

## 3. Execution provenance

### 3.1 Run metadata

```text
Run ID:
20260711T215410Z-focused-open-questions-v02

Start:
2026-07-11T21:54:10Z

Finish:
2026-07-11T21:56:05Z

Active duration:
115.240423 seconds

Configured maximum duration:
1,500 seconds

Time limit reached:
No
```

### 3.2 Source revision

```text
Expected commit:
f7eaffdd76140803b5796ab9c51955bdb0fb8e86

Recorded HEAD:
f7eaffdd76140803b5796ab9c51955bdb0fb8e86
```

The expected and recorded commits matched exactly.

The working tree was not completely clean:

```text
D docs/RESEARCH_STUDIES.md
?? docs/search/
```

The recorded modifications were under `docs/`. No source or public-header modification was reported by the captured status.

### 3.3 Tested library

```text
Library:
build/libctc_sigma.so.0.2.0

Version string:
0.2.0-dev

Library SHA-256:
223bbf06f259ab3d0b5a75999d3aa47ef5acd5df1e644e92534b694caf861733
```

This library hash matches the binary used by the earlier v0.2 audit and confirmation runs.

### 3.4 Environment

```text
Operating system:
Linux 7.0.14-101.fc43.x86_64

Architecture:
x86_64

Python:
CPython 3.14.6

Master seed:
4846281981894080333

Scale:
1.0
```

---

## 4. Evidence integrity

### 4.1 Result archive

```text
Archive:
20260711T215410Z-focused-open-questions-v02.zip

Archive SHA-256:
dce2893c205edc4c97d8fcc0221d018868b47d226dbea0b7535a18721f2928ce
```

The submitted `.sha256` file contained the same digest.

### 4.2 Manifest verification

```text
Manifest entries:
14

Entries with matching size:
14

Entries with matching SHA-256:
14

Manifest mismatches:
0
```

The archive is internally consistent.

### 4.3 Preserved evidence

The result package contains:

```text
environment.json
manifest.json
native_probe.json
native_probe_compile.log
native_probe_stderr.log
native_probe_stdout.csv
report.json
report.md
raw/encoder_exact_relations.csv
raw/encoder_affine_relations.csv
raw/sbox_local_symmetry.csv
raw/sbox_propagation.csv
raw/braid_ablation.csv
raw/capacity_toy_models.csv
```

The compiled native probe executable was also preserved.

---

# 5. Executive result

| Study area | Result | General security conclusion |
|---|---|---|
| Full v0.2 encoder relations | No tested exact, constant-delta, or affine relation survived all four normative subrounds | No full-encoder weakness confirmed |
| Reduced one-subround encoder | Exact affine relations were confirmed for every tested cross-tweak group | Reduced-round structural relation confirmed |
| Local S-box symmetry | Exact central symmetry confirmed in every case | Local algebraic property confirmed |
| One-subround ARITH symmetry | Exact constant-sum and affine relation confirmed | Reduced-round ARITH relation confirmed |
| Two or more ARITH subrounds | No tested constant-sum or affine relation survived | Simple tested relation not observed later |
| Braid-layer avalanche contribution | No predeclared measurable effect above the 0.5 percentage-point threshold | General braid security contribution remains unproven |
| Reduced sponge-capacity models | Partial interfaces changed mappings, but hidden cardinality did not behave as automatically equivalent to conventional capacity | Production effective capacity remains unresolved |
| Native C probe | Compiled and passed | Previous test-harness failure resolved |

---

# 6. Encoder relation study

## 6.1 Objective

The encoder study searched for residual structure between different tweak values of:

$$
A_{\mathrm{ENC},i,h}.
$$

The specific question was whether the v0.2 redesign still allowed a relation comparable to the v0.1 input/block-counter alias, or a different exact relation that survived the complete four-subround encoder permutation.

---

## 6.2 Tested configuration

### Rounds

```text
0, 3, 7, 11
```

### Source blocks for direct relation screening

```text
0, 1, 2, 3, 4, 7
```

### Related-input families

```text
add_one
subtract_one
power_two
balanced
negate
rotate
```

### Sample size

```text
Exact-relation groups:
144

Samples per exact-relation group:
64

Total direct related-input cases:
9,216
```

### Affine block-pair groups

The following block pairs were tested:

```text
0 → 1
1 → 2
2 → 3
3 → 4
0 → 4
```

for each of four rounds and each subround count from one through four:

```text
4 rounds × 5 block pairs × 4 subround counts = 80 affine groups
```

Each affine group used:

```text
64 total pairs
9 fitting pairs
55 independent holdout pairs
```

---

## 6.3 Direct exact-relation results

Across the 144 configured direct relation groups:

```text
Same-tweak exact equalities:
0

Next-tweak exact equalities:
0

Constant output-delta groups:
0
```

No direct tested relation recreated the removed v0.1 identity.

The tested neighboring tweak outputs also did not differ by one constant field vector across all samples.

### Interpretation

The following narrow statement is supported:

> None of the six tested related-input families produced an exact equality or sample-independent output delta between the configured encoder blocks.

This does not exclude every possible nonlinear, multiblock, or chosen-state relation.

---

## 6.4 Encoder inverse verification

The suite verified forward/inverse recovery across:

```text
4 rounds
6 block indices
4 subround counts
8 random inputs per combination
```

Total:

```text
768 encoder inverse checks
```

Result:

```text
Inverse failures:
0
```

Therefore:

$$
A_{\mathrm{ENC},i,h}^{-1}
\left(
A_{\mathrm{ENC},i,h}(x)
\right)
=
x
$$

held in every configured case.

---

## 6.5 Construction of aligned cross-tweak inputs

For a source block $h$ and target block $k$, the suite derived an input offset intended to align the input of the first nonlinear operation.

For lane $j$, the offset was:

$$
\Delta_j
=
RC_{h,j}
+
C_{h,j}
-
RC_{k,j}
-
C_{k,j}
\pmod q.
$$

The target input was then:

$$
x'_j=x_j+\Delta_j\pmod q.
$$

This ensures that the first Dickson input is aligned:

$$
x'_j+RC_{k,j}+C_{k,j}
=
x_j+RC_{h,j}+C_{h,j}
\pmod q.
$$

The two first-subround nonlinear cores therefore receive the same field value.

Because the remaining operations of that subround are affine or linear around that aligned nonlinear value, an affine output relation is structurally possible.

---

## 6.6 Confirmed one-subround affine relation

Results by subround count:

| Encoder subrounds | Affine groups | Universal on all independent holdouts |
|---:|---:|---:|
| 1 | 20 | 20 |
| 2 | 20 | 0 |
| 3 | 20 | 0 |
| 4 | 20 | 0 |

For one subround:

```text
Groups confirmed:
20 of 20

Independent holdout validations per group:
55

Total exact independent validations:
1,100
```

The relation was reproduced:

- for every tested round;
- for every tested block pair;
- for every independent holdout pair.

The resulting form is:

$$
A_{\mathrm{ENC},i,k}^{(1)}
\left(
x+\Delta_{h,k}
\right)
=
L_{i,h,k}
\left(
A_{\mathrm{ENC},i,h}^{(1)}(x)
\right)
+
d_{i,h,k},
$$

where $L_{i,h,k}$ and $d_{i,h,k}$ are public affine parameters determined by the two tweak domains.

### Classification

```text
One-subround cross-tweak affine relation:
CONFIRMED
```

This is a real reduced-round structural property.

---

## 6.7 Failure of the tested affine relation after later subrounds

For two, three, and four subrounds:

```text
Universal affine groups:
0 of 20 for each subround count
```

The fitted affine transformation did not validate on the independent holdout samples.

The second nonlinear layer therefore destroyed the tested affine structure.

### Normative encoder result

The v0.2 encoder uses four subrounds.

For the complete normative encoder:

```text
Exact tested relation:
Not observed

Constant-delta tested relation:
Not observed

General fitted affine relation:
Not observed
```

### Correct conclusion

> The v0.2 encoder has an exact one-subround cross-tweak affine relation, but the relation did not survive the second nonlinear subround and was not observed in the complete four-subround encoder.

---

## 6.8 Security interpretation

The confirmed reduced-round relation is relevant to security-margin analysis because it shows that:

- one encoder subround is structurally insufficient;
- the first subround alone does not hide all cross-tweak structure;
- the security margin depends on the later nonlinear subrounds.

The study did not demonstrate:

- factor-stream prediction from the affine relation;
- equal complete encoder outputs;
- equal Garside normal forms;
- equal FOLD outputs;
- a branch distinguisher;
- a permutation distinguisher;
- a Hash256 or XOF attack.

### Disposition

```text
Reduced one-subround relation:
CONFIRMED

Complete four-subround relation:
NOT OBSERVED IN THE TESTED FAMILY

Immediate encoder correction:
NOT REQUIRED BY THIS RESULT ALONE

Research significance:
RETAIN AS REDUCED-ROUND STRUCTURAL EVIDENCE
```

---

# 7. S-box symmetry propagation study

## 7.1 Objective

The S-box study had two goals:

1. confirm the exact local central symmetry in the production C implementation;
2. determine how far the induced relation propagates through complete ARITH subrounds.

The local relation is:

$$
S(-2C-x)=2A-S(x).
$$

Equivalently:

$$
S(x)+S(-2C-x)=2A.
$$

---

## 7.2 Local symmetry coverage

The configured groups covered:

```text
Labels:
A_PRE
A_FOLD
A_POST

Rounds:
0, 3, 7, 11

Subround indices:
0, 1, 2, 3

Lanes:
0 through 7
```

Total groups:

```text
3 labels × 4 rounds × 4 subrounds × 8 lanes
= 384 groups
```

Samples per group:

```text
48
```

Total local equality checks:

```text
384 × 48 = 18,432
```

Result:

```text
Exact local symmetry matches:
18,432 of 18,432

Groups with all samples exact:
384 of 384
```

### Classification

```text
Local central S-box symmetry:
CONFIRMED
```

---

## 7.3 One-subround ARITH partner construction

The first complete ARITH subround applies:

1. lane round-constant addition;
2. the S-box;
3. the MDS matrix.

For an input lane $x_j$, the suite constructed:

$$
x'_j
=
-x_j
-
2RC_j
-
2C_j
\pmod q.
$$

After round-constant addition, the two S-box inputs become central partners.

The two lane outputs then satisfy:

$$
S_j(x_j+RC_j)
+
S_j(x'_j+RC_j)
=
2A_j.
$$

Let:

$$
a=
(2A_0,\ldots,2A_7).
$$

Because the MDS layer is linear:

$$
M y+M y'
=
M(y+y')
=
Ma.
$$

The complete one-subround output sum is therefore constant.

---

## 7.4 Confirmed one-subround ARITH relation

Configured one-subround groups:

```text
3 labels × 4 rounds = 12 groups
```

Samples per group:

```text
64
```

Total one-subround pairs:

```text
768
```

Result:

```text
Constant output-sum matches:
768 of 768

Groups with the exact expected constant:
12 of 12

Universal affine holdout relations:
12 of 12
```

### Classification

```text
One-subround ARITH constant-sum relation:
CONFIRMED
```

This relation is an exact mathematical consequence of the local S-box symmetry and the linear MDS layer.

---

## 7.5 Propagation after two or more subrounds

The following subround counts were tested:

```text
2
3
4
6
```

For each count:

```text
3 labels × 4 rounds = 12 groups
64 samples per group
```

Total later-stage groups:

```text
48
```

Total later-stage pairs:

```text
3,072
```

Results:

```text
Later constant-sum groups:
0

Later universal affine groups:
0
```

For each later-stage group, the 64 tested output sums were distinct.

### Interpretation

The second nonlinear subround destroyed the simple constant-sum relation.

A fitted general affine map also failed to validate on the independent holdout samples.

### Correct conclusion

> The exact S-box symmetry propagates through one complete ARITH subround, but no constant-sum or general affine relation of the tested form survived two or more subrounds.

---

## 7.6 What this result does not exclude

The study did not search exhaustively for:

- nonlinear output relations;
- higher-degree algebraic relations;
- differential trails;
- integral properties;
- multi-pair relations;
- invariant subspaces;
- relations involving different partner constructions;
- Feistel-level propagation;
- Hash256 or XOF distinguishers.

Therefore:

```text
All possible uses of the symmetry are eliminated:
NOT PROVEN
```

---

## 7.7 Security-margin interpretation

The result demonstrates a meaningful round transition:

```text
1 subround:
exact exploitable-looking affine structure exists

2 or more tested subrounds:
the simple structure is no longer observed
```

This supports the importance of using multiple nonlinear subrounds.

It does not prove that the selected four- or six-subround ARITH configurations have sufficient cryptographic margin.

### Disposition

```text
Local S-box symmetry:
CONFIRMED

One-subround ARITH propagation:
CONFIRMED

Simple affine propagation after two or more subrounds:
NOT OBSERVED

Immediate S-box redesign:
NOT JUSTIFIED BY THIS STUDY ALONE

Further differential and algebraic analysis:
REQUIRED
```

---

# 8. Braid-layer contribution study

## 8.1 Objective

The braid-layer study investigated whether replacing the production Garside normal form with an empty valid normal form produced a measurable difference in selected branch diffusion metrics.

This is an ablation study.

It does not evaluate every possible cryptographic contribution of the braid layer.

---

## 8.2 Compared variants

### Production branch

```text
A_PRE
→ encoder factor generation
→ Garside normalization
→ FOLD
→ A_POST
```

### Empty-normal-form variant

The same branch path was used, but the injected normalizer returned:

```text
infimum = 0
factor_count = 0
```

This representation is accepted as a valid empty normal form.

The remainder of the branch, including FOLD and A_POST, still executed.

The variant therefore removes the semantic contribution of the normalized braid word while preserving the surrounding code path.

---

## 8.3 Difference families

Two related-input families were tested.

### Add-one family

One input lane was incremented by one modulo $q$.

### Chosen post-A_PRE symmetry family

The test selected a pair of states at the output of `A_PRE` that satisfied the first-subround central partner construction, and then inverted `A_PRE` to obtain corresponding valid branch inputs.

This is a chosen-state structural family rather than a random one-bit perturbation.

---

## 8.4 Coverage

```text
Independent seed groups:
8

Rounds:
0, 3, 7, 11

Difference families:
2

Cases per group/round/family:
64

Total cases:
4,096
```

For every case, the study measured:

- production related-input Hamming fraction;
- production independent-control Hamming fraction;
- empty-normal-form related-input Hamming fraction;
- empty-normal-form independent-control Hamming fraction;
- production-versus-empty output Hamming fraction;
- exact equality;
- changed lane count.

---

## 8.5 Mean results

### Production branch

$$
\bar h_{\mathrm{full}}
=
0.5005323066.
$$

Equivalent percentage:

```text
50.0532307%
```

Approximate 95% interval:

```text
49.9837286% to 50.1227327%
```

### Empty-normal-form branch

$$
\bar h_{\mathrm{empty}}
=
0.4999139504.
$$

Equivalent percentage:

```text
49.9913950%
```

Approximate 95% interval:

```text
49.9234633% to 50.0593267%
```

### Paired difference

$$
\bar h_{\mathrm{full}}
-
\bar h_{\mathrm{empty}}
=
0.0006183562.
$$

Equivalent percentage-point difference:

```text
+0.0618356 percentage point
```

Approximate 95% interval:

```text
-0.0358460 to +0.1595173 percentage point
```

The interval includes zero.

---

## 8.6 Predeclared practical threshold

The suite used:

```text
0.005 Hamming fraction
```

which equals:

```text
0.5 percentage point
```

as the minimum effect size for a predeclared measurable contribution in this experiment.

The observed paired difference was:

```text
0.000618
```

which is approximately:

```text
12.4% of the predeclared threshold
```

and the interval crossed zero.

---

## 8.7 Direction across independent groups

### Add-one family

```text
Positive full-minus-empty group means:
5

Negative group means:
3
```

### Chosen post-A_PRE symmetry family

```text
Positive group means:
4

Negative group means:
4
```

No stable direction was observed across the groups.

---

## 8.8 Exact output comparison

```text
Cases in which production output equaled empty-normal-form output:
0 of 4,096
```

Therefore, the braid contribution changes the branch function.

The result is not that the braid layer has no effect.

The result is that its removal did not create a sufficiently large or consistent change in the selected average Hamming metrics.

---

## 8.9 Correct interpretation

The study supports the following statement:

> Within 4,096 cases, two selected related-input families, four tested rounds, and the configured branch Hamming metrics, the semantic braid contribution produced no predeclared measurable effect above 0.5 percentage point.

The study does not support:

```text
The braid layer is useless.
The braid layer is redundant.
The braid layer adds no security.
The braid layer should be removed.
The empty-normal-form variant is cryptographically equivalent.
```

---

## 8.10 Why the general security contribution remains unresolved

Average Hamming distance does not directly measure:

- maximum differential probability;
- integral behavior;
- invariant destruction;
- algebraic degree;
- interpolation complexity;
- collision multiplicity;
- internal state merging;
- structural distinguishers;
- SAT/SMT complexity;
- Gröbner-basis complexity;
- preimage resistance.

A component may have little effect on ordinary avalanche while still affecting one of these properties.

Conversely, changing avalanche does not itself prove that a component adds meaningful security.

### Disposition

```text
Braid layer changes the function:
CONFIRMED

Braid contribution above the selected Hamming threshold:
NOT OBSERVED

General cryptographic contribution:
INCONCLUSIVE

Redundancy:
NOT DEMONSTRATED
```

---

# 9. Reduced sponge-capacity model study

## 9.1 Objective

The capacity study investigated a specific modeling question:

> May unexposed high portions of partially serialized rate coordinates be counted automatically as conventional sponge capacity?

The production CTC-Σ state uses field lanes of approximately 61 bits, while each rate lane absorbs and emits 40 serialized bits.

A simple cardinality argument might count the unexposed high portions as additional capacity.

The reduced models were designed to test whether partial interfaces behave identically to an ordinary sponge merely because more state bits remain hidden.

---

## 9.2 Reduced model parameters

Each model used:

```text
State size:
12 bits

Declared rate coordinate:
6 bits

Declared conventional capacity:
6 bits

Number of absorption blocks:
2

Permutation:
independently shuffled random permutation

Instances:
20
```

All messages in each reduced input space were enumerated exhaustively.

---

## 9.3 Model variants

### Conventional model

```text
Input bits per block:
6

Output bits:
6
```

### Partial input and partial output

```text
Input bits per block:
4

Output bits:
4
```

### Partial input and wider output

```text
Input bits per block:
4

Output bits:
6
```

---

## 9.4 Aggregate results

| Model | Messages | Mean distinct outputs | Mean maximum preimages | Mean collision pairs |
|---|---:|---:|---:|---:|
| Conventional 6-in/6-out | 4,096 | 64.00 | 83.20 | 131,064.6 |
| Partial 4-in/4-out | 256 | 16.00 | 23.80 | 2,047.0 |
| Partial 4-in/6-out | 256 | 62.85 | 9.55 | 515.7 |

---

## 9.5 Interpretation

The reduced models showed that changing the input and output alphabets materially changed:

- the number of reachable outputs;
- the average and maximum output multiplicity;
- the number of collision pairs;
- the mapping structure.

The hidden-state cardinality alone did not determine these properties.

Therefore:

> State bits that are not directly serialized cannot automatically be assigned the complete security role of conventional capacity without analyzing how the restricted interface interacts with the permutation.

---

## 9.6 Important model limitations

The reduced experiment used:

- a 12-bit state;
- XOR-style injection;
- random finite permutations;
- two absorption blocks;
- direct truncation;
- no complete CTC-Σ rejection mechanism;
- no production field arithmetic;
- no full CTC-Σ padding or domain encoding;
- no production branch or Feistel structure.

It does not reproduce the production construction exactly.

The study therefore cannot establish:

```text
244-bit effective security
256-bit effective security
328-bit effective security
488-bit formal capacity
656-bit formal capacity
```

---

## 9.7 Production claim implication

For the production parameters:

$$
q=2^{61}-1,
$$

$$
c_{\mathrm{lane}}
=
8\log_2(q)
\approx488\text{ bits},
$$

and:

$$
c_{\mathrm{lane}}/2
\approx244\text{ bits}.
$$

The restricted serialized interface creates additional hidden internal cardinality, but this study does not justify treating all of it as conventional capacity.

The defensible conclusion remains:

> The conventional eight-capacity-lane argument does not by itself justify an unqualified 256-bit preimage or second-preimage claim, and the restricted 40-bit interface requires a construction-specific analysis before a stronger claim can be made.

### Disposition

```text
Hidden cardinality automatically equals conventional capacity:
NOT SUPPORTED

Production effective capacity:
UNRESOLVED

244-bit exact security:
NOT PROVEN

256-bit exact security:
NOT PROVEN

328-bit alternative security:
NOT PROVEN

Formal analysis:
REQUIRED
```

---

# 10. Native C probe

## 10.1 Purpose

The previous targeted test package contained a probe that failed to compile because a signedness warning was promoted to an error by `-Werror`.

The current study used corrected probe code.

---

## 10.2 Compilation

Compiler options:

```text
-std=c11
-O2
-Wall
-Wextra
-Werror
```

Result:

```text
Compile return code:
0

Warnings promoted to errors:
None

Probe compiled:
Yes
```

---

## 10.3 Execution

Result:

```text
Run return code:
0

encoder_inverse:
PASS

local_sbox_symmetry:
PASS

stderr:
empty
```

The probe independently confirmed:

- encoder forward/inverse recovery for its configured cases;
- exact local S-box central symmetry.

### Disposition

```text
Previous probe failure:
RESOLVED

Current native probe:
PASS

Failure attributed to CTC-Σ production code:
NO
```

---

# 11. Consolidated findings

## 11.1 Confirmed findings

### `F-FOQ-001` — One-subround encoder affine relation

Cross-tweak inputs can be aligned so that every tested one-subround encoder group satisfies an exact affine relation.

```text
Status:
CONFIRMED
```

### `F-FOQ-002` — Local S-box central symmetry

The production S-box satisfies:

$$
S(-2C-x)=2A-S(x)
$$

for every tested configuration.

```text
Status:
CONFIRMED
```

### `F-FOQ-003` — One-subround ARITH constant-sum relation

The S-box partner construction propagates exactly through one complete ARITH subround because the MDS layer is linear.

```text
Status:
CONFIRMED
```

### `F-FOQ-004` — Later nonlinear subrounds break the tested affine relations

No configured encoder or ARITH affine relation validated after the second nonlinear subround.

```text
Status:
NOT OBSERVED AFTER TWO OR MORE SUBROUNDS
```

### `F-FOQ-005` — Braid semantics change branch outputs

The production branch and empty-normal-form branch produced different outputs in every case.

```text
Status:
CONFIRMED
```

### `F-FOQ-006` — No selected braid Hamming effect above threshold

The paired average effect was below the predeclared 0.5 percentage-point threshold and lacked stable direction.

```text
Status:
NO MEASURABLE EFFECT IN THE TESTED METRICS
```

### `F-FOQ-007` — Partial exposure is not automatically ordinary capacity

The exhaustive toy models showed that interface restrictions alter finite mappings in ways not captured by hidden-bit counting alone.

```text
Status:
MODEL EVIDENCE CONFIRMED
```

### `F-FOQ-008` — Corrected native probe execution

The strict native probe compiled and passed.

```text
Status:
CONFIRMED
```

---

# 12. Findings not established

The study did not establish:

- a practical full-encoder relation;
- an exact relation after four normative encoder subrounds;
- propagation of the simple S-box relation after two or more subrounds;
- absence of all higher-degree or nonlinear relations;
- a practical branch or permutation distinguisher;
- a measurable general security contribution from the braid layer;
- redundancy of the braid layer;
- effective preimage resistance of 244, 256, or 328 bits;
- collision or preimage attacks;
- production readiness.

---

# 13. Correction relevance

## 13.1 Encoder

No immediate encoder correction is required solely from this study.

The one-subround relation should be documented and retained as a reduced-round security-margin finding.

Recommended actions:

1. preserve four normative encoder subrounds;
2. do not reduce the encoder to one subround;
3. add a regression test for the confirmed reduced relation;
4. continue searching for nonlinear and factor-stream relations;
5. test whether the one-subround relation affects reduced branch variants.

---

## 13.2 S-box and ARITH

The exact local symmetry is intrinsic to the selected S-box form.

The current study found that the second nonlinear subround breaks the simple tested affine relation.

Recommended actions:

1. document the symmetry as a known algebraic property;
2. preserve more than one ARITH subround;
3. perform differential and integral propagation analysis;
4. evaluate whether the symmetry improves reduced-round trails;
5. avoid claiming that the symmetry is harmless solely because the affine relation disappears.

A redesign is not justified by this study alone.

---

## 13.3 Braid layer

The braid layer changed every tested output but did not produce a measurable improvement in the selected Hamming metrics.

Recommended actions:

1. do not remove the braid layer based only on avalanche evidence;
2. do not claim a security contribution without attack-oriented evidence;
3. compare production and no-braid variants using:
   - differential probabilities;
   - algebraic degree;
   - integral properties;
   - reduced collision searches;
   - solver complexity;
   - structural invariants;
4. define explicit security responsibilities for the braid layer.

The current status is a design-justification problem, not a confirmed defect.

---

## 13.4 Capacity claim

The capacity issue cannot be resolved by a longer version of the same empirical test.

Recommended options remain:

1. qualify or reduce the preimage and second-preimage claims;
2. increase conventional capacity;
3. provide a construction-specific proof for the restricted field interface.

Until one of these is completed, 256-bit preimage strength should remain an unproven design goal rather than an established property.

---

# 14. Recommended follow-up studies

## 14.1 Encoder reduced-round propagation

Test the confirmed one-subround affine relation through:

```text
factor decoding
factor rejection
32-factor stream generation
Garside normalization
FOLD
branch output
one to three Feistel rounds
```

The main question is whether the exact field relation produces any non-random downstream statistic.

---

## 14.2 S-box symmetry cryptanalysis

Perform:

- differential distribution analysis in larger reduced fields;
- boomerang and second-order differential screening;
- integral tests using central partner sets;
- symbolic degree tracking;
- SAT/SMT or Gröbner experiments on reduced ARITH;
- chosen-state propagation through branch and Feistel rounds.

---

## 14.3 Braid contribution analysis

Replace average avalanche as the primary metric with:

- maximum differential probabilities;
- normal-form multiplicity;
- internal collision counts in reduced models;
- algebraic equation counts and degree;
- solver runtime with and without braids;
- invariant searches;
- chosen-input distinguishers;
- FOLD output dependence.

---

## 14.4 Sponge proof model

Prepare a formal document defining:

1. the complete state space;
2. the message-block alphabet;
3. the exact absorption map;
4. the accepted squeeze-state set;
5. the output map;
6. the adversary's access to the permutation and inverse;
7. generic state-binding and path-finding costs;
8. collision, preimage, and second-preimage bounds.

This work is required before assigning a formal security meaning to the unexposed high portions of the first eight lanes.

---

# 15. Final assessment

This focused study resolved several previously ambiguous observations.

It confirmed that the selected S-box has an exact local central symmetry and that this symmetry creates an exact relation through one complete ARITH subround.

It also confirmed an exact affine cross-tweak relation in the encoder when reduced to one subround.

In both cases, the second nonlinear subround destroyed the simple tested relation, and no corresponding affine relation was observed in the normative four-subround encoder or in later ARITH configurations.

The braid-layer experiment was expanded to 4,096 cases. The braid semantics changed every branch output, but the selected average Hamming metrics showed no stable effect above the predeclared 0.5 percentage-point threshold. This does not establish redundancy; it means that the braid layer's security contribution remains unproven by these metrics.

The reduced capacity models showed that partially hidden state cardinality cannot automatically be treated as conventional sponge capacity. They did not determine the effective security level of production CTC-Σ. The 244-, 256-, and 328-bit interpretations remain theoretical questions requiring formal analysis.

The corrected native probe compiled and passed, resolving the previous test-harness failure.

The appropriate conclusion is:

```text
Reduced-round structural relations:
CONFIRMED

Simple relation in normative encoder:
NOT OBSERVED

Simple S-box relation after multiple ARITH subrounds:
NOT OBSERVED

Braid impact on selected avalanche metrics:
NO PREDECLARED MEASURABLE EFFECT

General braid security contribution:
UNRESOLVED

Production sponge capacity:
UNRESOLVED

Practical break of CTC-Σ:
NOT DEMONSTRATED
```

---

# Appendix A — Primary numerical results

## A.1 Encoder

```text
Direct relation groups:                    144
Samples per direct group:                   64
Direct related-input cases:              9,216
Exact next-tweak equalities:                 0
Constant-delta groups:                       0

Affine groups:                              80
One-subround universal affine groups:       20
Two-subround universal affine groups:        0
Three-subround universal affine groups:      0
Four-subround universal affine groups:       0

Encoder inverse checks:                    768
Encoder inverse failures:                    0
```

## A.2 S-box and ARITH

```text
Local symmetry groups:                     384
Samples per local group:                    48
Local exact checks:                     18,432
Local failures:                              0

One-subround propagation groups:            12
Samples per group:                           64
Exact one-subround pairs:                   768
One-subround failures:                       0

Later propagation groups:                   48
Later constant-sum groups:                   0
Later universal affine groups:               0
```

## A.3 Braid ablation

```text
Cases:                                   4,096

Production related-input mean:
0.5005323066

Empty-normal-form related-input mean:
0.4999139504

Paired mean difference:
0.0006183562

Predeclared effect threshold:
0.005

Production/empty exact equalities:
0
```

## A.4 Capacity toy models

```text
Instances per model:
20

State bits:
12

Declared rate-coordinate bits:
6

Absorption blocks:
2
```

| Variant | Mean distinct outputs | Mean maximum preimages | Mean collision pairs |
|---|---:|---:|---:|
| 6-in/6-out | 64.00 | 83.20 | 131,064.6 |
| 4-in/4-out | 16.00 | 23.80 | 2,047.0 |
| 4-in/6-out | 62.85 | 9.55 | 515.7 |

---

# Appendix B — Evidence files

```text
report.json
    Machine-readable aggregate conclusions.

report.md
    Automatically generated short report.

raw/encoder_exact_relations.csv
    Exact equality, lane equality, constant-delta, and Hamming observations.

raw/encoder_affine_relations.csv
    Affine-fit and holdout validation results.

raw/sbox_local_symmetry.csv
    Exact local central-symmetry checks.

raw/sbox_propagation.csv
    Constant-sum and affine propagation results by subround count.

raw/braid_ablation.csv
    Per-case production and empty-normal-form branch metrics.

raw/capacity_toy_models.csv
    Exhaustive reduced-model output multiplicity data.

native_probe_compile.log
    Exact native compilation command and diagnostics.

native_probe_stdout.csv
    Native test results.

environment.json
    Runtime environment and command-line configuration.

manifest.json
    File sizes and SHA-256 values.
```

---

# Appendix C — Research status update

The following project-level statuses are supported by this study:

| Research item | Updated status |
|---|---|
| v0.2 full encoder simple relation | Not observed in tested families |
| One-subround encoder affine structure | Confirmed |
| Local S-box central symmetry | Confirmed |
| One-subround ARITH propagation | Confirmed |
| Multi-subround simple affine propagation | Not observed |
| Braid contribution to average avalanche | No measurable effect above configured threshold |
| General braid security contribution | Open |
| Capacity interpretation | Open; formal analysis required |
| Native focused probe | Pass |
