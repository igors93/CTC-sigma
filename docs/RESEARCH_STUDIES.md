# CTC-Σ Research and Evaluation Record

**Document status:** Living technical record  
**Project:** CTC-Σ  
**Purpose:** Track the design, execution, evidence, conclusions, limitations, and next actions of every research phase applied to the CTC-Σ reference implementation.

> **Security notice**
>
> CTC-Σ remains an experimental cryptographic construction. Passing the studies described in this document does not prove cryptographic security and does not authorize production use. The results only describe what was observed within the tested scope, software revision, hardware, compiler, configuration, seeds, and sample sizes.

---

## 1. How to use this document

This file is the central research guide for the project. It must be updated whenever:

- a new phase is designed;
- an experiment is executed;
- a previous result is repeated with a larger sample;
- a result changes after a code or specification modification;
- a limitation is removed or a new limitation is discovered;
- a conclusion is strengthened, weakened, or withdrawn.

Detailed raw evidence remains in `research/results/`. This document records the technical interpretation of that evidence.

The source of truth for a specific execution is always the generated result package containing:

```text
config.json
metadata.json
summary.json
report.md
failures.jsonl
warnings.jsonl
manifest.json
raw/
matrices/
```

Not every phase produces every optional directory, but every final conclusion must be traceable to a preserved result archive.

---

## 2. Result vocabulary

The following terms have precise meanings throughout this document.

### PASS

No acceptance threshold was violated in the tested scope.

A `PASS` is not a proof of security. It means that the experiment behaved as expected for the tested sample and configuration.

### REVIEW

A statistically unusual, structurally unexpected, or insufficiently reproduced observation was found.

A `REVIEW` result is not automatically a vulnerability. It requires targeted confirmation or rejection.

### FAIL

A required invariant, implementation contract, reproducibility condition, or experiment acceptance criterion was violated.

A `FAIL` must be investigated before the affected phase can be considered complete.

### INCOMPLETE

The study could not execute all required experiments, environments, toolchains, or sample sizes.

### BLOCKED

A required test could not run because of an environmental or tooling limitation rather than a confirmed problem in CTC-Σ.

---

## 3. Research principles

All studies must follow these principles:

1. **Reproducibility**  
   Every run must record the seed, source hashes, compiler, flags, operating system, CPU, build type, library hash, and command-line configuration.

2. **Separation of correctness and security**  
   Functional agreement does not imply cryptographic security. Statistical quality does not imply collision, preimage, or differential resistance.

3. **Preservation of raw evidence**  
   Summaries alone are insufficient. Raw observations, matrices, warnings, failures, and manifests must be retained.

4. **No silent exclusion**  
   Failed cases, rejected samples, unavailable tools, interrupted runs, and incomplete environments must remain visible in the final report.

5. **Independent confirmation where possible**  
   Independent algorithms, implementations, compilers, seeds, and platforms should be used whenever feasible.

6. **Multiple-testing control**  
   Statistical tests over many bits, lanes, pairs, rounds, or message classes must apply an appropriate correction such as false discovery rate control.

7. **No security claim from negative results**  
   Failure to find a weakness only means that no weakness was found by the executed experiments.

---

## 4. Current phase status

| Phase | Scope | Status | Main conclusion |
|---|---|---:|---|
| Phase 0A | Functional correctness | PASS | No functional divergence was found in the tested environment. |
| Phase 0B | Repeatability with multiple seeds | PASS | Repeated executions were deterministic and internally consistent. |
| Phase 0C | GCC versus Clang reproducibility | PASS | Evidence hashes matched for equivalent runs. |
| Phase 0D | Rejection-boundary contracts | PASS | Forced acceptance and rejection boundaries behaved as specified. |
| Phase 0E | Evidence integrity and traceability | PASS | Generated archives and manifests were internally consistent. |
| Phase 0F | ASan and UBSan execution | BLOCKED | Sanitizer runtimes were unavailable in the tested Fedora environment. |
| Phase 0 overall | Correctness and reproducibility baseline | INCOMPLETE | Functionally successful, but sanitizer execution remains pending. |
| Phase 1 | Statistical behavior and diffusion | PASS | General behavior was strong; the isolated Hash256 bit-159 observation was not reproduced by the confirmatory study. |
| Phase 1B | Hash256 anomaly confirmation | PASS | Across 20,000 paired perturbations and 10 seeds, bit 159 showed no significant or consistent deviation from 50%. |
| Phase 2A | Algebraic and structural baseline | REVIEW / INCOMPLETE | Reduced-field differential structure, S-box symmetry, braid contribution, production KAT, and solver confirmation remain open. The original encoder observation is superseded by Phases 2C and 2D. |
| Phase 2B | Production differential diffusion and simple symmetries | PASS | An eight-hour run completed 332,241 paired cases; all tested local differences reached near-50% diffusion by round 3 or earlier, and no tested global transformation commuted with the final permutation. |
| Phase 2C | Encoder related-input dependence confirmation | REVIEW | A 160,000-case study confirmed 64 lane-aggregate correlations only for the add-one family. No normal-form equality or FOLD output-bit bias was confirmed. |
| Phase 2D | Encoder localization and production-path propagation | REVIEW; propagation PASS | The direct effect was localized to source lane 0. Independent analysis confirmed an exact input/counter block-shift identity. An ordinary add-one perturbation before `A_PRE` produced no confirmed downstream effect in the tested path. |
| Phase 2 overall | Differential, structural, and algebraic cryptanalysis | REVIEW / INCOMPLETE | Strong diffusion evidence remains intact, but the confirmed encoder input/counter aliasing, S-box questions, braid contribution, and algebraic-solver work are unresolved. |
| Phase 3 | Performance analysis | NOT STARTED | Preliminary measurements indicate that the reference implementation is slow. |
| Phase 4 | Comparison with other algorithms | NOT STARTED | Pending stable benchmark methodology. |
| Phase 5 | Implementation security | PARTIALLY STARTED | Static CI exists; dedicated fuzzing and successful sanitizer runs remain pending. |
| Phase 6 | Final assessment | NOT STARTED | Depends on all previous phases. |

---

# 5. Phase 0 — Correctness, repeatability, and reproducibility

## 5.1 Objective

Phase 0 establishes whether the implementation is coherent enough to support later scientific analysis.

It does not evaluate cryptographic strength. It evaluates:

- deterministic behavior;
- known-answer vector stability;
- forward/inverse consistency;
- agreement between independently written normalization algorithms;
- agreement between GCC and Clang builds;
- exact rejection-boundary behavior;
- evidence integrity;
- environment capture.

---

## 5.2 Main experiments

Phase 0 included the following experiment groups.

### Known-answer vectors

Verified:

- Hash256 outputs for message lengths `0, 1, 39, 40, 41, 80, 1024`;
- XOF outputs for multiple lengths, including `1, 31, 32, 40, 41, 64, 1000`;
- permutation vectors for zero and counter states;
- permutation inverse recovery.

### Determinism and XOF prefix consistency

Repeated calls were checked for byte-identical outputs.

The XOF was also checked for prefix consistency:

$$
\mathrm{XOF}(M,n+k)[0:n] = \mathrm{XOF}(M,n)
$$

### Permutation reversibility

For sampled states:

$$
P_\Sigma^{-1}(P_\Sigma(X)) = X
$$

### ARITH reversibility

For all configured arithmetic labels and subround counts:

$$
\mathrm{ARITH}^{-1}(\mathrm{ARITH}(X)) = X
$$

### Lehmer rank and unrank

The complete space of `8! = 40320` permutations was checked:

$$
\mathrm{Rank}(\mathrm{Unrank}(r)) = r
$$

### Garside normalization agreement

The C normalizer and an independently written Python normalizer were compared on random signed words.

The comparison included:

- infimum;
- canonical factor count;
- factor sequence;
- projection to $S_8$;
- proper-factor range.

### Pipeline with independent normalizer

Hash256 and XOF were executed through the C pipeline while replacing only the C Garside normalizer with the independent Python normalizer.

This is an independent-normalizer test, not a fully independent implementation of the complete CTC-Σ algorithm.

### Forced rejection boundaries

Exact acceptance and rejection limits were tested directly for:

- 32-bit candidate extraction;
- factor-index rejection;
- 40-bit squeeze acceptance.

### Compiler reproducibility

Equivalent Release runs were executed with GCC and Clang.

Deterministic evidence hashes were compared between compilers.

---

## 5.3 Phase 0 evidence summary

A completed matrix produced:

```text
GCC Release   × 3 independent seeds
Clang Release × 3 independent seeds
```

All six Release runs completed successfully.

The combined recorded volume was:

| Experiment | Recorded observations | Failures |
|---|---:|---:|
| Known-answer vectors | 138 | 0 |
| Rejection contracts | 126 | 0 |
| Determinism | 48 | 0 |
| Permutation round-trip | 72 | 0 |
| ARITH round-trip | 1,500 | 0 |
| Lehmer exhaustive checks | 241,920 | 0 |
| Garside C versus Python | 600 | 0 |
| Pipeline with independent normalizer | 36 | 0 |
| Encoder telemetry | 600 | 0 |
| Sponge telemetry | 36 | 0 |
| **Total** | **245,076** | **0** |

For every matching seed, GCC and Clang produced identical deterministic evidence hashes.

---

## 5.4 Phase 0 conclusions

### Confirmed within the tested scope

- Known-answer vectors were stable.
- Hash256 and XOF were deterministic.
- XOF prefix consistency held.
- Permutation inversion recovered every tested state.
- ARITH inversion recovered every tested branch vector.
- Lehmer rank/unrank passed exhaustive verification.
- C and Python Garside normalizers agreed for every tested word.
- The complete C pipeline produced identical Hash256 and XOF outputs when driven by the Python normalizer.
- Forced rejection limits behaved as expected.
- GCC and Clang produced equivalent functional evidence.
- Result manifests were internally consistent.

### Not established

- Complete independent reimplementation of all CTC-Σ components.
- Cross-architecture reproducibility.
- Big-endian behavior.
- Memory safety under successful ASan and UBSan execution.
- Resistance to collision, preimage, differential, algebraic, or structural attacks.
- Production suitability.

---

## 5.5 Phase 0 sanitizer limitation

The sanitizer matrix did not execute because the Fedora environment could not link the required runtime files:

```text
libasan.so.8.0.0
libubsan.so.1.0.0
```

This was an environment failure during toolchain validation, before executing the CTC-Σ test workload.

Therefore:

```text
Functional result: PASS
Sanitizer result:   BLOCKED
Phase 0 overall:    INCOMPLETE
```

Phase 0 may be formally closed after successful AddressSanitizer and UndefinedBehaviorSanitizer runs, or after an equivalent documented environment is used.

---

# 6. Phase 1 — Statistical behavior and diffusion

## 6.1 Objective

Phase 1 evaluates whether small input differences spread through the permutation and output functions in a manner compatible with a strong cryptographic mixing function.

It analyzes:

- avalanche by round;
- strict avalanche behavior;
- lane influence;
- pairwise bit dependence;
- structured-state diffusion;
- Hash256 and XOF avalanche;
- output-bit balance;
- byte distribution;
- serial and lane correlations;
- braid-layer distributions.

Phase 1 does not establish collision or preimage security.

---

## 6.2 Execution summary

Three independent Phase 1 runs were completed.

```text
Run 0: PASS
Run 1: PASS
Run 2: REVIEW
```

The third run was classified as `REVIEW` only because of one isolated Hash256 output-bit observation.

The full matrix duration was approximately:

```text
21 minutes 35 seconds
```

---

## 6.3 Permutation avalanche

Final-round mean avalanche values were:

```text
Run 0: 49.9645%
Run 1: 50.0040%
Run 2: 50.0361%
```

Combined mean:

$$
50.0015\%
$$

No output bit remained significant after false discovery rate correction.

### Interpretation

For a one-bit input difference, the final permutation changed almost exactly half of the 976 state bits on average.

This is consistent with strong diffusion in the tested sample.

---

## 6.4 Diffusion by round

The approximate round progression was:

| Round | Mean changed-state fraction |
|---:|---:|
| 1 | about 12.7% |
| 2 | about 37.6% |
| 3 | about 50.0% |
| 4–12 | remained near 50.0% |

### Interpretation

The first round still exposes the expected balanced-Feistel structure.

By the third round, the observed state difference is already close to the ideal 50% level.

The remaining rounds maintain that behavior without visible degradation or periodic loss of diffusion.

This is evidence of rapid diffusion, not proof that three rounds are cryptographically secure.

---

## 6.5 Lane influence

No lane was persistently weak in the final state.

After approximately three rounds, all sixteen lanes approached 50% influence.

Later rounds generally remained near:

```text
49.7% to 50.3%
```

No permanent left/right Feistel imbalance was observed in the final output.

---

## 6.6 Pairwise bit dependence

The largest absolute sampled BIC correlations were approximately:

```text
0.08135
0.07360
0.07988
```

Combined mean maximum:

```text
about 0.0783
```

The mean correlations were close to zero, positive and negative extremes were balanced, and the same pair did not persist as an extreme across seeds.

### Conclusion

No strong pairwise dependence was identified in the sampled pairs.

The experiment sampled only a fraction of all possible bit pairs and must not be interpreted as exhaustive.

---

## 6.7 Lane correlation

The largest absolute lane correlations were:

```text
0.06188
0.06642
0.05602
```

Combined mean maximum:

```text
about 0.06144
```

No lane pair repeatedly showed a strong relationship across seeds.

---

## 6.8 Structured-state diffusion

The experiment included structured inputs such as:

- ascending and descending counters;
- alternating zero and $q-1$;
- equal Feistel halves;
- powers of two;
- arithmetic progressions;
- symmetric and repeated patterns.

The same general progression was observed:

```text
Round 1: partial diffusion
Round 2: approximately 35%–40%
Round 3: approximately 50%
Round 12: approximately 49%–51%
```

Representative final results included:

```text
reverse_counter:        49.93%
alternating_zero_max:   49.24%
equal_halves:           49.91%
powers_of_two:          50.08%
arithmetic_progression: 50.35%
```

### Conclusion

No simple tested structured state preserved an obvious symmetry after three rounds.

This does not exclude unknown invariant subspaces.

---

## 6.9 Hash256 avalanche

Mean Hash256 avalanche values were:

```text
Run 0: 49.8138%
Run 1: 49.9837%
Run 2: 50.0112%
```

Combined mean:

$$
49.9363\%
$$

For the reviewed run:

```text
comparisons:        384
mean changed bits:  128.03 of 256
minimum:            105
maximum:            157
standard deviation: 8.26
```

For an ideal binomial model:

$$
E[X] = 128
$$

$$
\sigma = 8
$$

The observed mean and standard deviation were close to those values.

---

## 6.10 Hash256 bit 159 observation

In one of the three runs, output bit `159` changed with frequency:

```text
60.9375%
```

The minimum frequency among all output bits in the same run was:

```text
41.1458%
```

After false discovery rate correction over the 256 Hash256 output bits, bit 159 remained significant in that run with an adjusted probability of approximately:

```text
0.00464
```

### Why this is not yet classified as a vulnerability

- It did not repeat in the other two seeds.
- Each output bit had only 384 observations.
- Many output bits were tested simultaneously.
- No related anomaly appeared in the permutation SAC result.
- No equivalent anomaly appeared in XOF.
- The observed direction has not yet been reproduced across independent seeds.

### Current classification

```text
Isolated, non-reproduced statistical anomaly
```

The observation triggered Phase 1B.

### Subsequent resolution

Phase 1B later tested this exact pre-registered hypothesis with 20,000 paired one-bit perturbations across ten independent seeds. The bit changed in 9,950 of 20,000 Hash256 comparisons, corresponding to 49.75%, with $p=0.483905$. The 99.9% Wilson interval included 50%, and the seed directions were split six below and four above 50%.

The Phase 1 observation is therefore retained as part of the historical record but is classified as:

```text
CLOSED — NOT REPRODUCED
```

It is no longer an open statistical anomaly.

---

## 6.11 XOF avalanche

Mean 256-bit XOF avalanche values were:

```text
Run 0: 49.9430%
Run 1: 49.9776%
Run 2: 50.0793%
```

Combined mean:

$$
50.0000\%
$$

No persistent output-bit anomaly comparable to Hash256 bit 159 was observed.

---

## 6.12 Output distribution

Hash256 byte-distribution chi-square probabilities included:

```text
0.5876
0.1052
0.1938
```

No run produced an extreme result indicating a clear byte-frequency bias.

Bit-frequency, runs, and serial-correlation tests also found no persistent anomaly.

No collision was observed in the generated samples.

The collision observation is only a sanity check because the sample size is far too small to evaluate 256-bit collision resistance.

---

## 6.13 Braid-layer distributions

Each seed analyzed approximately:

```text
5,000 normalized words
```

Total:

```text
15,000 normalized words
```

### Sign balance

Sign-balance probabilities were:

```text
0.5320
0.3953
0.2420
```

No sign imbalance was detected.

### Infimum

Mean infimum values were:

```text
-12.0080
-12.0568
-12.0096
```

Combined mean:

$$
-12.0248
$$

Representative range:

```text
minimum: -23
maximum: -1
median:  -12
standard deviation: 3.23
```

The negative infimum is expected because the generated word contains approximately half negative factors.

### Canonical length

Mean canonical factor counts were:

```text
24.0660
24.0998
24.1014
```

Combined mean:

$$
24.0891
$$

Representative distribution:

```text
minimum: 15
maximum: 31
median:  24
standard deviation: 2.31
```

### Drop and Keep

Representative averages were:

```text
Keep: 15.9998
Drop:  8.1016
```

The 16-factor Keep window was therefore almost always full.

This is not automatically a weakness because Drop factors are also encoded into the fold input. However, it means that `c_G = 16` acts as a real cut in almost every sampled normal form.

Reduced-window and knockout experiments should compare at least:

```text
8, 16, 24, 32
```

retained factors during Phase 2.

---

## 6.14 Phase 1 conclusion

### Confirmed within the tested scope

- Final permutation avalanche was essentially 50%.
- The permutation reached approximately 50% avalanche by round 3.
- No persistent weak lane was detected.
- No statistically persistent output bit was detected in the permutation.
- No strong sampled BIC or lane correlation was detected.
- Simple structured states did not preserve visible symmetry.
- Hash256 and XOF had near-ideal mean avalanche.
- Output byte and bit distributions did not show a stable bias.
- Braid-layer distributions were stable across seeds.

### Historical review item and resolution

Phase 1 originally received `REVIEW` because Hash256 output bit 159 changed in 60.9375% of 384 comparisons in one of three runs and remained significant after the correction used in that run.

That observation was not reproduced by the larger, pre-registered Phase 1B experiment. The original result remains documented in Section 6.10, but its disposition is now:

```text
F1-001 — Hash256 output bit 159
CLOSED — NOT REPRODUCED
```

### Remaining limitations

- Sampled BIC coverage was not exhaustive.
- Statistical testing was performed on one machine and one principal architecture.
- The Keep window was saturated in nearly every sampled normal form and still requires structural comparison during Phase 2.
- Statistical quality does not establish collision, preimage, differential, algebraic, or structural security.

### Phase result

```text
Phase 1: PASS within the tested statistical scope
```

This classification means that no statistical anomaly remained confirmed after the planned follow-up. It does not constitute a cryptographic security claim.

# 7. Phase 1B — Hash256 bit-anomaly confirmation

## 7.1 Objective

Phase 1B was a focused confirmatory study designed to determine whether the Phase 1 observation at Hash256 output bit 159 was:

1. a reproducible output bias;
2. a bias restricted to certain message lengths or input-bit locations;
3. a Hash256-domain effect not present in XOF;
4. or an ordinary statistical fluctuation.

The hypothesis and decision criteria were defined before the Phase 1B result was interpreted.

---

## 7.2 Pre-registered standard experiment

The standard Phase 1B profile specified:

```text
10 independent seeds
2,000 one-bit message perturbations per seed
20,000 total comparisons
```

For every comparison, the study computed:

```text
Hash256(original message)
Hash256(one-bit-modified message)
XOF-256(original message)
XOF-256(one-bit-modified message)
```

The study recorded:

- message length;
- changed input-bit index;
- changed bit position within the byte;
- relative input position;
- complete Hash256 difference mask;
- complete XOF difference mask;
- Hamming distance;
- state of Hash256 output bit 159;
- state of XOF output bit 159;
- per-seed and combined counts.

Boundary-focused lengths included:

```text
39, 40, 41
79, 80, 81
```

These lengths were selected to examine behavior around the 40-byte rate boundary.

---

## 7.3 Pre-registered confirmation criteria

The bit-159 observation would be classified as confirmed only if all of the following held:

1. the combined two-sided test had:

$$
p < 0.001
$$

2. the 99.9% Wilson confidence interval excluded 50%;

3. the deviation had the same direction in at least 7 of 10 independent seeds.

Additional evidence would be considered stronger if the effect:

- repeated at the same message lengths;
- repeated for the same relative input positions;
- remained significant after global false discovery rate correction;
- appeared in Hash256 but not XOF under matched inputs;
- remained visible after excluding individual seed groups.

These criteria were not changed after observing the results.

---

## 7.4 Execution provenance

The completed run recorded:

```text
run id:             20260710T211042Z
profile:            standard
seeds:              10
comparisons/seed:   2,000
total comparisons: 20,000
duration:           approximately 2 h 37 min 59 s
platform:           Fedora Linux 43, x86_64
CPU:                AMD Athlon 3000G
Python:             CPython 3.14.6
build type:         Release
compiler family:    GCC
core commit:        c73fd4bea6a5e90e5be18e7b8bf36193be8d2d50
core tree:          clean
research tree:      untracked under research/
```

The tested shared library was:

```text
SHA-256:
18080dc792a85a0cde06eaa8f91b34837657c691a126746f86f402511ee4253f
```

The preserved result archive was:

```text
20260710T211042Z.zip
SHA-256:
c41a730d9979b6f01274fb1c1757317813bfe724a584aeae291092e2179202d2
```

The research-source aggregate hash recorded by the run was:

```text
047bc033ec182f5fe7fcf839ca2f506bc4f94e41553ba78f00035f5b8df65d2b
```

The archive contained 26 manifest entries. All recorded sizes and SHA-256 values were verified. `failures.jsonl` and `warnings.jsonl` were empty.

---

## 7.5 Primary Hash256 bit-159 result

The target output bit changed in:

```text
9,950 of 20,000 comparisons
```

Therefore:

$$
\hat p = 0.4975 = 49.75\%
$$

The deviation from 50% was:

```text
-0.25 percentage point
```

The pre-registered two-sided test produced:

```text
p = 0.483905
```

The 99.9% Wilson confidence interval was:

```text
48.5871% to 50.9132%
```

The interval includes 50% comfortably.

### Confirmation-criterion decision

| Criterion | Required | Observed | Met? |
|---|---:|---:|---:|
| Combined p-value | below 0.001 | 0.483905 | No |
| 99.9% interval | exclude 50% | includes 50% | No |
| Common direction | at least 7 of 10 seeds | 6 negative, 4 positive | No |

None of the three required criteria was satisfied.

---

## 7.6 Behavior across seeds

The Hash256 bit-159 change frequencies were:

| Seed index | Frequency |
|---:|---:|
| 0 | 51.55% |
| 1 | 49.50% |
| 2 | 49.30% |
| 3 | 47.75% |
| 4 | 51.80% |
| 5 | 50.85% |
| 6 | 48.40% |
| 7 | 49.15% |
| 8 | 48.15% |
| 9 | 51.05% |

The observed range was:

```text
47.75% to 51.80%
```

No seed approached the original 60.9375% observation.

The direction count was:

```text
below 50%: 6 seeds
above 50%: 4 seeds
```

The two-sided sign-test probability was:

```text
p = 0.753906
```

The between-seed heterogeneity test produced:

```text
p = 0.078558
```

Neither result supports a consistent seed-dependent effect.

---

## 7.7 Comparison with XOF

For the matched 256-bit XOF output, bit 159 changed in:

```text
10,091 of 20,000 comparisons
frequency: 50.455%
p-value:   0.200593
```

Its 99.9% Wilson interval was:

```text
49.2917% to 51.6178%
```

The paired Hash256-versus-XOF McNemar comparison produced:

```text
p = 0.161031
```

No significant target-bit difference was found between the Hash256 and XOF domains.

---

## 7.8 Global avalanche behavior

### Hash256

```text
mean changed bits:  128.06065 of 256
mean percentage:    50.02369%
median:             128
standard deviation: 8.00265
minimum:            96
maximum:            160
1st percentile:     109
99th percentile:    147
```

### XOF-256

```text
mean changed bits:  128.03230 of 256
mean percentage:    50.01262%
median:             128
standard deviation: 8.00188
minimum:            97
maximum:            162
1st percentile:     110
99th percentile:    147
```

For an ideal binomial model with 256 output bits:

$$
E[X]=128,\qquad \sigma=8.
$$

Both observed distributions were extremely close to those reference values.

---

## 7.9 Exploratory all-bit analysis

The confirmatory target was bit 159. The study also explored all output bits.

After Benjamini-Hochberg correction with $q=0.01$:

```text
pooled Hash256 and XOF tests: 512
rejections:                    0

per-seed bit tests:            5,120
rejections:                    0

paired Hash256-vs-XOF tests:   256
rejections:                    0
```

Some nominally small uncorrected probabilities appeared, as expected when hundreds or thousands of hypotheses are examined. Examples included:

```text
XOF bit 157:
frequency ≈ 51.175%
unadjusted p ≈ 0.000912

Hash256 bit 2:
frequency ≈ 50.960%
unadjusted p ≈ 0.006765
```

Neither remained significant after the pre-specified multiple-testing correction.

These observations are exploratory and are not classified as findings.

---

## 7.10 Message-length analysis

For Hash256 bit 159, the global heterogeneity test across twenty message-length classes produced:

```text
p = 0.028422
```

This was a secondary analysis and did not meet the primary confirmation criteria.

The most notable boundary-class observation was:

```text
40-byte messages:
frequency = 54.7%
n = 1,000
unadjusted p = 0.00327
```

Neighboring classes were:

```text
39 bytes: 52.2%
40 bytes: 54.7%
41 bytes: 50.7%
```

A conservative Bonferroni threshold for twenty message-length classes at a 5% family-wise level is:

$$
0.05/20 = 0.0025.
$$

The 40-byte result did not cross that threshold.

Another exploratory class, 63 bytes, produced a nominal Hash256-versus-XOF paired probability near 0.00169. It likewise arose within a broad secondary search and did not alter the confirmatory conclusion.

### Interpretation

No message-length-specific anomaly is confirmed.

The 40-byte and 63-byte observations may remain in the historical record as low-priority exploratory items, but they are not sufficient to keep Phase 1 open.

---

## 7.11 Decision

The Phase 1B confirmatory study did not reproduce the previously observed Hash256 output-bit 159 anomaly.

The formal disposition is:

```text
Phase 1B: PASS

F1-001 — Hash256 output bit 159:
CLOSED — NOT REPRODUCED

Phase 1:
PASS within the tested statistical scope
```

The recommended scientific statement is:

> Across 20,000 paired one-bit perturbations and ten independent seeds, Hash256 output bit 159 changed with frequency 49.75%. The result was not significantly different from 50%, the 99.9% confidence interval included 50%, the seed directions were inconsistent, and no significant paired difference from XOF was detected. The isolated Phase 1 observation is therefore classified as a non-reproduced statistical fluctuation.

---

## 7.12 What Phase 1B does not prove

Phase 1B does not establish:

- absolute absence of output bias;
- collision resistance;
- preimage or second-preimage resistance;
- differential or integral resistance;
- adequacy of twelve rounds;
- a security contribution from the braid layer;
- production suitability.

It establishes only that the specific, pre-registered bit-159 anomaly was not reproduced with substantially greater statistical power.

# 8. Phase 2A — Algebraic and structural baseline

## 8.1 Objective

Phase 2A establishes an initial algebraic and structural baseline for the current CTC-Σ design.

The study was designed to identify simple mathematical inconsistencies, low-degree invariants, related-input structures, reduced-round diffusion behavior, and evidence about the contribution of individual components. It does not constitute a complete algebraic attack and does not establish security against differential, integral, interpolation, Gröbner-basis, SAT/SMT, or structural cryptanalysis.

The evaluated experiment groups were:

- exact field and Dickson-permutation checks;
- exact S-box equations and central symmetry;
- exhaustive MDS-minor verification in a reduced field;
- sampled ARITH Jacobian-rank analysis;
- exhaustive reduced-field S-box differential spectra;
- linear and quadratic invariant searches;
- structured-subspace preservation searches;
- Mersenne-related input symmetries;
- encoder behavior under related inputs;
- braid relations, projection consistency, and normal-form sensitivity;
- component-knockout avalanche comparisons;
- reduced-round avalanche without the braid layer;
- polynomial-system export;
- optional production KAT and Gröbner experiments.

---

## 8.2 Execution provenance and scope

The recorded standard-profile execution used:

```text
profile:          standard
seed:             1
duration:         9.872620 seconds
platform:         Linux x86_64
Python:           CPython 3.14.6
recorded branch:  main
recorded commit:  3c833468f6c4ff2dbf517550d6dda48e6712d4f4
working tree:     dirty
```

The execution produced:

```text
PASS:     10
REVIEW:    3
SKIPPED:   2
TOTAL:    15
```

Two result directories with the same profile and seed were preserved. Their substantive metrics were identical. This demonstrates deterministic repeatability for that configuration, but it is not independent confirmation because both executions used the same seed, implementation, machine class, and experiment design.

The tests were executed from the CTC-Σ project tree and modeled the current design parameters. However, the production-permutation known-answer test was disabled in this profile. Consequently, this run alone does not establish bit-for-bit equivalence between every Python research model and the current compiled C implementation.

The working tree was recorded as dirty. The exact research-suite archive and result archive must therefore be preserved together with their hashes if the execution is cited as scientific evidence.

---

## 8.3 Experiment status summary

| Experiment | Scope | Status | Main observation |
|---|---|---:|---|
| `field_and_dickson_exact` | Field and Dickson criteria | PASS | No inconsistency found; degrees 23 and 47 satisfy the permutation criterion. |
| `sbox_equations_and_symmetry` | S-box bijection and exact identities | PASS | Bijection held; exact central symmetry was confirmed. |
| `mds_exact` | All square minors in the reduced field | PASS | 12,869 minors checked; no zero minor found. |
| `arith_jacobian` | Sampled Jacobian ranks | PASS | All 128 sampled Jacobians had full rank 8. |
| `sbox_differential_spectrum` | Exhaustive reduced-field DDT | REVIEW | Differential uniformities 14 and 12 exceeded the sampled random baseline maximum of 10. |
| `low_degree_invariant_search` | Linear and quadratic invariants | PASS | No verified invariant found in the tested reduced model. |
| `structured_subspace_search` | Five structured state families | PASS | No tested family was preserved. |
| `mersenne_related_input_symmetry` | Rotations/scalings and negation | PASS | No exact simple relation found; distances remained near 50%. |
| `encoder_related_inputs` | Related encoder blocks | REVIEW | Maximum observed sign correlation was 0.41072, but the sample was small. |
| `braid_relations_and_sensitivity` | Artin relations and normal-form response | PASS | No relation or projection failure; no random normal-form collision observed. |
| `knockout_avalanche` | Component-removal diffusion | REVIEW | Removing the braid normalization produced almost the same two-round mean avalanche in four samples. |
| `reduced_round_avalanche` | Reduced rounds without braids | PASS | Mean avalanche reached approximately 50% by round 3. |
| `production_permutation_kat` | Full model versus production vectors | SKIPPED | Disabled in the standard profile. |
| `export_polynomial_system` | Algebraic-model generation | PASS | A 20-variable, 16-equation model was exported. |
| `optional_sage_groebner` | Gröbner-basis solving | SKIPPED | SageMath execution was disabled. |

---

## 8.4 Exact field, Dickson, S-box, and diffusion-layer results

### Field and Dickson permutations

The production modulus was recorded as:

$$
q=2^{61}-1=2305843009213693951.
$$

The implementation reported it as probably prime, and the permutation criteria for both selected Dickson degrees were satisfied:

$$
\gcd(23,q^2-1)=1,
\qquad
\gcd(47,q^2-1)=1.
$$

In the reduced exhaustive field $\mathbb F_{1279}$, both maps produced all 1,279 possible outputs exactly once.

No failure was recorded.

### S-box equations and exact central symmetry

For both degrees, all 2,558 tested reduced-field inputs satisfied the expected S-box equations, and each S-box produced 1,279 unique outputs.

The exact relation

$$
S(-2C-x)=2A-S(x)
$$

held across the complete tested reduced field.

This is not an implementation failure. It is an exact algebraic property caused by the use of odd-degree Dickson polynomials together with inversion and affine transformations. It is classified as cryptanalytically relevant because it may support related-input distinguishers, affine-symmetry propagation, or integral constructions if it survives composition with later layers.

No zero-derivative point was observed in the exhaustive reduced-field test.

### MDS verification

The complete set of square minors of the $8\times8$ reduced-field matrix was checked:

```text
1×1 minors:  64
2×2 minors:  784
3×3 minors:  3,136
4×4 minors:  4,900
5×5 minors:  3,136
6×6 minors:  784
7×7 minors:  64
8×8 minors:  1
total:       12,869
```

No zero minor was found, and the matrix rank was 8. Within the tested field, this confirms the expected MDS property.

### ARITH Jacobian

The Jacobian of the tested arithmetic layer had rank 8 in all 128 sampled points:

```text
full-rank samples: 128
singular samples:    0
```

This result did not reveal a sampled local algebraic degeneracy. It remains an empirical result rather than a proof over every input and every production-field configuration.

---

## 8.5 Differential spectrum of the S-boxes

The differential distribution tables were computed exhaustively over:

$$
\mathbb F_{1279}.
$$

The results were:

| Function | Differential uniformity | Maximum probability |
|---|---:|---:|
| Dickson degree 23 S-box core | 14 | $14/1279\approx1.0946\%$ |
| Dickson degree 47 S-box core | 12 | $12/1279\approx0.9382\%$ |
| Random permutation controls | 9, 10, 9 | maximum $10/1279\approx0.7819\%$ |

For degree 23, the maximum count 14 occurred for at least the following differential pairs:

```text
input difference 265  -> output difference 211
input difference 1014 -> output difference 1068
```

The degree-47 maximum count was 12 and occurred for multiple differential pairs.

### Interpretation

The result is a real property of the tested reduced S-boxes, not a sampling fluctuation inside the DDT, because the reduced-field differential tables were exhaustive.

However, the comparison baseline contained only three random permutations, and the field was reduced. The result therefore does not establish a practical differential attack on the production permutation.

The correct classification is:

```text
Structured differential behavior requiring confirmation and propagation analysis
```

Required follow-up work includes:

- repeat the exhaustive experiment over $\mathbb F_{8191}$;
- increase the random-permutation baseline;
- propagate the strongest differentials through one and multiple ARITH subrounds;
- measure active S-box counts under the MDS layer;
- test whether the central S-box symmetry improves differential or integral trails;
- compare full and no-braid branches under the same differential inputs.

---

## 8.6 Low-degree invariants and structured subspaces

The invariant search used:

```text
field:                 F_127
lanes:                 4
ARITH subrounds:       2
maximum degree:        2
training samples:      512
verification samples:  512
```

No linear or quadratic invariant candidate survived:

```text
degree 1 candidate dimension: 0
degree 2 candidate dimension: 0
```

The following structured families were each tested for all 127 configured values:

- all lanes equal;
- alternating $x,-x$;
- arithmetic progression;
- palindrome;
- only lane 0 nonzero.

None was preserved.

These are positive negative results: no simple invariant or tested invariant family was found. They do not exclude higher-degree invariants, larger-lane structures, invariants spanning several Feistel rounds, or structures specific to the production field.

---

## 8.7 Mersenne-related input symmetries

The study tested the Mersenne-field-related transformations associated with powers of two and negation.

For rotations/scalings by:

```text
1, 2, 5, 7, 13, 31, 60
```

no exact match was found in 64 samples per transformation. Mean normalized Hamming distances remained close to 0.5.

The negation experiment also produced no exact match and had:

```text
mean normalized Hamming distance: 0.496734
minimum normalized distance:      0.467213
```

No simple rotational, scaling, or negation relation was identified in the tested arithmetic configuration.

This result reduces concern about an immediate Mersenne-rotation symmetry, but it does not exclude more complex affine relations or relations involving different output transformations.

---

## 8.8 Encoder behavior under related inputs

The encoder experiment evaluated 32 samples. Every sample generated exactly four blocks, consistent with the expected extraction of 32 factors in groups of eight.

Observed sensitivity under:

$$
u\mapsto u+e_0
$$

was:

```text
minimum changed factors: 32 of 32
mean changed factors:    32 of 32
unique sequences:        32 of 32
```

This is a positive sensitivity result.

The maximum observed correlations between related block positions were:

```text
maximum absolute sign correlation: 0.410720
maximum absolute rank correlation: 0.330816
```

The sign correlation caused the `REVIEW` classification.

### Limitation

The sample size was only 32, while multiple position pairs were examined. The result archive did not record a multiple-testing-corrected significance value. Therefore, the maximum observed correlation is insufficient by itself to establish a persistent encoder relation.

The finding must be treated as:

```text
Unconfirmed related-input correlation in a small sample
```

Required follow-up work includes at least 512 samples per seed, multiple independent seeds, confidence intervals, and false-discovery-rate or family-wise-error correction.

---

## 8.9 Braid relations and normal-form sensitivity

The braid experiment used 128 random signed words of length 32.

Results:

```text
Artin/commutation failures:        0
projection failures:              0
random normal-form collisions:    0
unique random normal forms:       128 of 128
```

Changing one factor produced:

```text
minimum changed factor positions: 4
mean changed factor positions:    17.8828125
maximum changed factor positions: 29
mean absolute infimum change:      0.765625
mean absolute length change:       0.875
```

These results support the functional consistency and local combinatorial sensitivity of the normalizer in the tested sample.

They do not estimate the global multiplicity of the map from encoded words to normal forms and do not prove that Garside normalization contributes a measurable security margin.

---

## 8.10 Component-knockout avalanche

The knockout study compared two-round avalanche using only four samples per variant.

| Variant | Mean normalized avalanche | Difference from full |
|---|---:|---:|
| Full | 0.447746 | — |
| No braid normalization | 0.448258 | +0.000512 |
| No pre-mix | 0.438525 | −0.009221 |
| No direct path | 0.371926 | −0.075820 |
| No fold | 0.314293 | −0.133453 |
| Raw factors | 0.318391 | −0.129355 |
| No post-mix | 0.303791 | −0.143955 |
| Degree 23 only | 0.490010 | +0.042264 |
| Degree 47 only | 0.485400 | +0.037654 |

The closest result to the full construction was the no-braid variant.

### Interpretation

Within this very small two-round avalanche sample, removing braid normalization did not reduce average diffusion. This suggests that the arithmetic layers already account for most of the observed avalanche.

This result does **not** prove that the braid layer is redundant. Avalanche measures diffusion only. The braid layer may still affect differential probabilities, algebraic model complexity, integral behavior, invariant destruction, collision structure, or distinguishers.

The result creates a research obligation:

> The braid layer must be justified through attack resistance or structural effects, not solely through ordinary avalanche measurements.

Because only four samples and two rounds were used, no design change should be made from this result alone.

---

## 8.11 Reduced-round behavior without the braid layer

The reduced-round experiment explicitly used the `no_braid` variant with eight samples.

| Rounds | Mean normalized avalanche |
|---:|---:|
| 1 | 0.157787 |
| 2 | 0.407787 |
| 3 | 0.500512 |
| 4 | 0.500897 |

The arithmetic and Feistel structure reached approximately 50% mean avalanche by round 3 even without braid normalization.

This reinforces the conclusion that fast diffusion is primarily supplied by the arithmetic layers and Feistel propagation. It does not determine whether the braid layer contributes protection against non-avalanche attacks.

---

## 8.12 Algebraic model export and skipped experiments

The polynomial export generated a reduced ARITH model with:

```text
variables:               20
equations:               16
maximum relation degree: 23
lanes:                    4
field:                    F_127
```

The export confirms that the S-box and arithmetic layer can be represented as a compact polynomial system with intermediate variables.

No Gröbner-basis solver was executed because the SageMath experiment was disabled. Consequently, no actual algebraic preimage, inversion, or reduced-round solving result was obtained.

The production-permutation KAT was also disabled. Therefore:

- the algebraic suite was not automatically checked against the complete production permutation vectors in this run;
- no statement should claim full model equivalence solely from this execution;
- the Gröbner attack remains designed but unexecuted.

---

## 8.13 Phase 2A conclusions

### Confirmed within the tested scope

- The selected Dickson degrees satisfied the required permutation criterion.
- The reduced S-boxes were bijective.
- The exact central symmetry of the S-boxes was confirmed.
- The reduced-field Cauchy matrix passed exhaustive MDS-minor verification.
- No singular Jacobian was observed in 128 samples.
- No tested linear or quadratic invariant was found.
- No tested structured subspace was preserved.
- No simple Mersenne rotation, scaling, or negation relation was found.
- In the small Phase 2A same-position comparison, all 32 factor positions changed under the tested $u+e_0$ perturbation. Phases 2C and 2D later showed that this does not imply factor-stream independence: for source lane 0, 24 factors can be shared exactly after an eight-position shift.
- No braid-relation, projection, or sampled normal-form collision failure occurred.
- The arithmetic/Feistel construction reached near-ideal avalanche by round 3 without braids.
- A machine-readable polynomial model was successfully exported.

### Findings requiring review

1. **Exact S-box central symmetry**  
   This is mathematically expected but may support structured attacks if it propagates.

2. **Differential uniformity above the sampled random baseline**  
   Uniformities 14 and 12 were observed in $\mathbb F_{1279}$, versus a sampled random maximum of 10.

3. **Encoder related-input correlation — historical precursor, superseded**  
   A maximum absolute correlation of 0.410720 was observed in the small Phase 2A sample. Phase 2C later confirmed a weaker but highly reproducible lane-aggregate add-one dependence. Phase 2D then localized the effect to source lane 0 and identified the exact input/counter block-shift identity described in Sections 10 and 11.

4. **Near-equivalent no-braid avalanche**  
   The no-braid variant had nearly the same two-round mean avalanche as the full branch in four samples.

### Not established

- Bit-for-bit agreement of the research model with the current production C permutation in this run.
- Gröbner-basis resistance.
- Differential resistance of complete ARITH, branch, Feistel, Hash256, or XOF constructions.
- Absence of higher-degree or full-state invariants.
- Absence of integral, interpolation, rotational, boomerang, rebound, or related-key-style distinguishers.
- A quantified security contribution from the braid layer.
- Adequacy of 12 rounds.
- Collision or preimage security.
- Production readiness.

### Phase result

```text
Phase 2A: REVIEW
Scope completion: INCOMPLETE
```

The original `REVIEW` result was caused by the differential spectrum and two then-unconfirmed structural observations. The encoder observation is no longer merely underpowered: it is superseded by the confirmed Phase 2C and Phase 2D findings. The scope remains incomplete because the production KAT and Gröbner experiments were skipped and the standard profile used reduced models and small samples for several experiments.

---


# 9. Phase 2B — High-power production differential validation

## 9.1 Objective

Phase 2B was designed as an eight-hour confirmatory production study.

Its primary question was:

> Do predefined local differences and simple global algebraic or lane transformations leave persistent, reproducible structure after the production 12-round permutation?

Unlike the reduced-field and reduced-lane experiments of Phase 2A, this study exercised the compiled production C library and tracked the propagation of differences through all twelve Feistel rounds.

The study evaluated:

- six local differential families;
- six global symmetry or commutation families;
- diffusion after every round;
- final Hamming-distance distributions;
- final per-bit frequencies;
- exact commutation counts;
- cross-seed persistence;
- agreement between the public branch-based round trace and the official production permutation.

This study did not attempt to calculate a formal differential-security bound.

---

## 9.2 Predeclared hypotheses and thresholds

The experiment registered the following hypotheses before interpreting the final data.

```text
H1  Every local differential family has a final mean changed-bit
    fraction in [0.49, 0.51].

H2  After one global false-discovery-rate correction across all final
    output-bit tests, no bit has q < 0.001, absolute effect >= 0.005,
    and the same direction in at least 8 of 10 seeds.

H3  Every local family enters and remains in [0.49, 0.51] by round 4.

H4  No tested global transformation commutes exactly with the final
    production permutation.

H5  Every final commutation residual has a mean changed-bit fraction
    in [0.49, 0.51] and no confirmed biased bit.

H6  The public branch trace agrees exactly with
    ctc_permutation_apply on every verification case.
```

The principal predeclared thresholds were:

```text
seed groups:                         10
minimum cases per seed/category:    1,000
global FDR threshold:               q = 0.001
minimum bit-bias effect:            0.5 percentage point
minimum consistent seed count:      8 of 10
maximum diffusion convergence:      round 4
allowed exact final commutations:   0
allowed production-trace mismatch:  0
```

A `PASS` under these rules means that no listed review or hard-failure condition was triggered. It is not a cryptographic-security proof.

---

## 9.3 Execution provenance and evidence integrity

The completed production run recorded:

```text
run id:                       20260711T002528Z
profile:                      standard
active duration:              8.000 hours
start time:                   2026-07-11 00:25:28 UTC
finish time:                  2026-07-11 08:25:29 UTC
paired task count:            332,241
complete cycles:              2,768
seed groups:                  10
categories:                   12
seed/category cells:          120
minimum cases in any cell:    2,768
maximum cases in any cell:    2,769
required minimum per cell:    1,000
final verdict:                PASS
```

The analyzed source state was:

```text
core commit:
e60c7d5c093b87485697241bd16138523f556a00

Git working tree:
clean

production library SHA-256:
18080dc792a85a0cde06eaa8f91b34837657c691a126746f86f402511ee4253f
```

The preserved result archive was:

```text
20260711T002528Z.zip

SHA-256:
2827983c41dbe4862ed559d7ed3fec404b0329c2b61da73d5c5971edc0bdba5d
```

Evidence-integrity checks produced:

```text
manifest entries verified: 49 of 49
manifest mismatches:        0
hourly snapshot archives:   9
corrupt snapshot archives:  0
failure records:            0
warning records:            0
```

The use of a clean working tree and a preserved library hash improves the traceability of this study relative to earlier exploratory runs.

---

## 9.4 Tested difference and transformation families

### Local differential families

The study used:

1. `single_bit_flip`  
   Flip exactly one valid state bit.

2. `add_one_lane`  
   Add one to one field lane.

3. `add_power_of_two_lane`  
   Add a selected power of two to one lane.

4. `negate_one_lane`  
   Replace one lane by its additive inverse.

5. `balanced_two_lane_delta`  
   Add $+\delta$ to one lane and $-\delta$ to another lane.

6. `mirrored_half_delta`  
   Apply the same delta to corresponding lanes in the two Feistel halves.

### Global transformation families

The study tested whether the production permutation retained or commuted with:

1. full-state negation;
2. multiplication of every lane by $2$;
3. multiplication of every lane by $2^{13}$;
4. swapping the two Feistel halves;
5. reversing lane order;
6. rotating lanes left by one position.

For a transformation $T$, two quantities were measured:

$$
P(x)\oplus P(T(x))
$$

and the commutation residual:

$$
P(T(x))\oplus T(P(x)).
$$

An exact zero residual would indicate exact commutation for that tested state.

---

## 9.5 Production round-trace consistency

The research runner reconstructed round progression through the public branch interface and compared its final result with the official production permutation.

The result was:

```text
verified states:   2,584
trace mismatches:  0
```

This supports the correctness of the round-by-round instrumentation used by Phase 2B.

It is not a fully independent implementation comparison because the trace and official path use components from the same compiled C library.

---

## 9.6 Local differential results

The observed changed-bit fractions were:

| Differential family | Round 1 | Round 2 | Round 3 | Round 12 | First round in 49%–51% band |
|---|---:|---:|---:|---:|---:|
| `single_bit_flip` | 12.5906% | 37.5376% | 49.9941% | 49.9986% | 3 |
| `add_one_lane` | 12.6213% | 37.5003% | 49.9937% | 49.9984% | 3 |
| `add_power_of_two_lane` | 12.6688% | 37.5722% | 50.0101% | 49.9964% | 3 |
| `negate_one_lane` | 17.2797% | 39.1234% | 49.9953% | 49.9862% | 3 |
| `balanced_two_lane_delta` | 19.4932% | 44.2786% | 50.0064% | 49.9937% | 3 |
| `mirrored_half_delta` | 25.2048% | 49.9932% | 49.9948% | 49.9959% | 2 |

### Interpretation

All six local families reached the predeclared 49%–51% band by round 3 or earlier.

The first two rounds continued to expose the expected Feistel propagation structure. By round 3, the mean Hamming behavior was close to the ideal 50% level and remained there through round 12.

No tested local family showed a later loss of diffusion.

The result confirms rapid empirical diffusion for these families. It does not establish that all high-probability differential trails are absent.

---

## 9.7 Global symmetry and commutation results

Final results were:

| Transformation | Final output difference | Final commutation residual | Exact final commutations | Confirmed residual-bit effects |
|---|---:|---:|---:|---:|
| `negate_all` | 50.0016% | 49.9984% | 0 | 0 |
| `scale_all_by_2` | 50.0085% | 50.0095% | 0 | 0 |
| `scale_all_by_2_power_13` | 49.9991% | 49.9936% | 0 | 0 |
| `swap_feistel_halves` | 50.0206% | 49.9986% | 0 | 0 |
| `reverse_lane_order` | 49.9860% | 50.0097% | 0 | 0 |
| `rotate_lanes_left_1` | 50.0191% | 49.9805% | 0 | 0 |

No tested transformation commuted exactly with the final production permutation.

Within the tested states, this rejects simple universal relations of the forms:

$$
P(-x)=-P(x),
$$

$$
P(2x)=2P(x),
$$

and:

$$
P(\mathrm{swap}(x))
=
\mathrm{swap}(P(x)).
$$

Several transformations retained visible partial structure in the first round, but their final output differences and final commutation residuals were near 50%.

---

## 9.8 Final distribution and output-bit analysis

The valid encoded state contains:

```text
976 bits
```

For an ideal independent bit model, the expected Hamming distance is:

$$
976/2=488
$$

with ideal binomial standard deviation:

$$
\sqrt{976\cdot0.5\cdot0.5}\approx15.6205.
$$

Across the eighteen final metrics—six local outputs, six symmetry output differences, and six symmetry commutation residuals—the observed values were:

```text
minimum final mean:             487.8093 bits
maximum final mean:             488.2007 bits
minimum final fraction:         49.98046%
maximum final fraction:         50.02057%
minimum observed std. dev.:     15.5279 bits
maximum observed std. dev.:     15.7289 bits
```

All sixteen lanes were affected in every recorded final metric sample.

### Global output-bit testing

The study performed:

```text
18 final metrics × 976 state bits = 17,568 tests
```

One global Benjamini–Hochberg correction was applied with:

```text
q = 0.001
```

The result was:

```text
rejections before effect filtering: 0
confirmed effects:                  0
replicated effects:                 0
```

No final output bit was classified as persistently biased under the predeclared rules.

---

## 9.9 Exploratory bit-929 observation

The most deviant pooled bit occurred in the final commutation residual for `swap_feistel_halves`:

```text
bit:                  929
frequency:            51.3111%
absolute deviation:   1.3111 percentage points
nominal p-value:      approximately 1.32 × 10^-5
within-metric adjusted p-value:
                      approximately 0.01048
required threshold:   below 0.001
```

This observation did not pass the predeclared multiple-testing threshold and was not classified as a confirmed effect.

Its correct status is:

```text
Exploratory candidate — not statistically confirmed
```

It may be retested in a future pre-registered study focused specifically on the same transformation and output bit. It does not change the Phase 2B `PASS` verdict.

---

## 9.10 Statistical scope and power limitation

Phase 2B had strong power for:

- large deviations in mean avalanche;
- slow convergence of the tested difference families;
- exact commutation;
- large persistent output-bit biases;
- broad lane failures;
- production trace mismatches.

However, the test family was large:

```text
17,568 simultaneous final bit hypotheses
```

After a very strict global correction, approximately 27,687 observations per pooled bit metric are not sufficient to exclude every isolated effect as small as the nominal 0.5-percentage-point threshold with high power.

Therefore, the correct conclusion is:

> No large, globally significant, and reproducible final output-bit effect was detected.

The correct conclusion is not:

> Every possible final bit bias greater than 0.5 percentage point has been excluded.

This distinction preserves the real statistical reach of the experiment.

---

## 9.11 What Phase 2B confirmed

Within the tested implementation, state encoding, sample design, and transformation families:

1. all six local differential families reached mean diffusion near 50% by round 3 or earlier;
2. the near-50% behavior remained stable through round 12;
3. final Hamming-distance means and standard deviations were close to an ideal binomial reference;
4. all lanes were active in final outputs;
5. none of the six global transformations commuted exactly with the final permutation;
6. no final output bit survived the predeclared global multiple-testing correction;
7. no production trace mismatch occurred in 2,584 verification states;
8. checkpointing and hourly snapshots preserved a complete eight-hour execution.

---

## 9.12 What Phase 2B did not resolve

This battery did not directly test:

- the complete differential distribution of the production S-boxes;
- the reduced-field uniformity values 14 and 12 from Phase 2A;
- formal propagation probabilities for the strongest S-box differentials;
- the exact central S-box relation:
  $$
  S(-2C-x)=2A-S(x);
  $$
- encoder related-input correlations;
- full versus no-braid attack resistance;
- the security contribution of Garside normalization;
- higher-degree invariants;
- SAT, SMT, Gröbner, or other algebraic solving;
- collision or preimage resistance;
- integral, rebound, boomerang, interpolation, or rotational attacks;
- all possible affine or non-linear symmetries.

Near-ideal average Hamming distance does not exclude a carefully structured high-probability differential.

---

## 9.13 Infrastructure observation

The final checkpoint recorded:

```text
snapshot_count:               9
last_snapshot_active_second:  0.0
```

All nine snapshot ZIP files were present and valid.

The zero value is therefore a metadata bookkeeping inconsistency rather than evidence loss. It should be corrected in a future runner revision, but it does not affect the completed Phase 2B measurements.

---

## 9.14 Phase 2B conclusion

### Confirmed within the tested scope

- rapid production-round diffusion;
- stable final near-50% changed-bit behavior;
- no tested exact final global commutation;
- no confirmed final output-bit bias;
- correct agreement between the round trace and official production output;
- complete checkpoint and snapshot preservation.

### Phase result

```text
Phase 2B:
PASS
```

### Effect on the overall Phase 2 status

```text
Phase 2 overall:
REVIEW / INCOMPLETE
```

Phase 2B strengthens the evidence that the full production permutation rapidly destroys the tested simple local differences and global symmetries.

The overall phase remains open because the following Phase 2A findings have not been resolved:

```text
F2A-001 — Exact local S-box central symmetry:
OPEN

F2A-002 — Reduced-field differential uniformity 14/12:
OPEN

F2A-003 — Encoder related-input correlation:
SUPERSEDED BY F2C-001 AND F2D-001

F2A-004 — Measurable cryptographic contribution of the braid layer:
OPEN

F2C-001 — Add-one raw-encoder lane-aggregate dependence:
CONFIRMED

F2D-001 — Encoder lane-0 input/block-counter aliasing:
CONFIRMED; COMPLETE-CONSTRUCTION IMPACT UNKNOWN

F2D-002 — Ordinary pre-A_PRE add-one propagation:
NOT OBSERVED WITHIN THE TESTED PRODUCTION PATH
```

The next confirmatory work should prioritize the exact encoder relation and its chosen-state propagation rather than repeat ordinary avalanche testing.

---

# 10. Phase 2C — Encoder related-input dependence confirmation

## 10.1 Objective

Phase 2C was designed to resolve the underpowered encoder-correlation observation from Phase 2A.

Its primary question was:

> Does the production encoder preserve reproducible dependence between predefined related inputs, and does any detected relation survive Garside normalization and `FOLD` when compared with independent controls?

The study tested four related-input families:

- add one to one lane;
- add a power of two to one lane;
- add a balanced two-lane delta;
- negate one lane.

The tested round indices were `0`, `3`, `7`, and `11`.

---

## 10.2 Predeclared confirmation criteria

An encoder correlation was classified as confirmed only when all of the following conditions held:

```text
Benjamini-Hochberg q-value:                 < 0.001
absolute related correlation:               >= 0.05
absolute excess over independent control:   >= 0.03
consistent direction:                       at least 8 of 10 seed groups
```

The study separately corrected:

- encoder position and lane correlation families;
- normal-form comparisons;
- `FOLD` output-bit tests.

This design was intended to distinguish reproducible related-input structure from ordinary finite-sample noise.

---

## 10.3 Execution provenance and integrity

```text
Run ID:                 20260711T122146Z
Profile:                standard
Started:                2026-07-11T12:21:46Z
Finished:               2026-07-11T12:57:23Z
Duration:               2134.861 seconds
Completed cases:        160,000 / 160,000
Seed groups:            10
Round indices:          0, 3, 7, 11
Transforms:             4
Cases per round/family: 10,000
Master seed:            317952932156252027433539
```

Each case evaluated a base input, a related input, and an independent control through the encoder, Garside normalization, and `FOLD`. The run therefore represented approximately 480,000 complete encoder/normalization/FOLD evaluations.

Implementation provenance:

```text
Core commit:
0f514333610ce74bace424305c586320e9775b52

Configuration SHA-256:
2f29e16bd27c4ab47af5500778c060e8f23eac97fc4885e7ec701b2116ea1bd8

Production library SHA-256:
18080dc792a85a0cde06eaa8f91b34837657c691a126746f86f402511ee4253f

Result ZIP SHA-256:
61f33d9c5ff5c517a426279b0b7314ca8159ec2b5cf1349b39df8b5c16591ec3
```

Environment:

```text
CPU:       AMD Athlon 3000G with Radeon Vega Graphics
Platform:  Linux x86_64
Python:    CPython 3.14.6
Git tree:  clean
```

Integrity results:

```text
Manifest entries valid:  16 / 16
Snapshots valid:          2 / 2
Failures:                 0
Warnings:                 0
Checkpoints decoded:      A and B
```

---

## 10.4 Confirmed raw-encoder dependence

The study executed:

```text
Encoder correlation tests:       3,072
Confirmed encoder correlations:  64
```

All 64 confirmed findings belonged exclusively to:

```text
transform = add_one_lane
```

No confirmed encoder correlation was found for:

- `add_power_of_two_lane`;
- `balanced_two_lane_delta`;
- `negate_one_lane`.

The confirmed metrics were only the lane-aggregate metrics:

```text
lane_sign
lane_rank
```

No individual `position_sign` or `position_rank` comparison satisfied the complete confirmation rule.

The confirmed matrix indices were:

```text
0, 9, 18, 27, 36, 45, 54, 63
```

These are the diagonal cells of an $8\times8$ lane matrix. At the Phase 2C level, this indicated that aggregating the four factor contributions associated with the same lane retained a small related-input dependence.

Across the 64 confirmed findings:

```text
related correlation range:        0.073122 to 0.121647
mean related correlation:         0.093101
excess-over-control range:        0.065306 to 0.113627
mean excess over control:         0.087007
10/10 consistent seed groups:     57 findings
9/10 consistent seed groups:       7 findings
```

The effect occurred at every tested round index.

### Relation to the Phase 2A observation

Phase 2A had observed a maximum correlation of approximately `0.410720` with only 32 samples.

Phase 2C established that:

- the original magnitude was substantially overestimated by the small sample;
- the underlying phenomenon was nevertheless real;
- the reproducible aggregate effect was closer to approximately `0.07`–`0.12`;
- the effect was specific to the add-one family within this experiment.

---

## 10.5 Factor sensitivity, normal form, and FOLD

Despite the aggregate correlation, same-position factor sensitivity remained high:

```text
mean changed factors:                 31.9993 to 31.9999 of 32
all 32 same-position factors changed: 99.93% to 99.99%
exact encoder words equal:            0
```

This same-position result must not be interpreted as proof that the two factor streams share no factors. Phase 2D later showed that an eight-position shift exposes exact factor sharing for source lane 0.

After Garside normalization:

```text
exact normal forms equal:             0
positional similarity range:          0.0001840 to 0.0002964
```

No related-versus-control normal-form effect met the predeclared confirmation criteria.

After `FOLD`:

```text
exact FOLD outputs equal:             0
mean Hamming fraction range:          0.499354 to 0.500427
all eight output lanes changed:       100% of cases
FOLD output-bit tests:                7,808
confirmed FOLD bit effects:           0
```

Encoder generation required:

```text
4 blocks: 159,969 related cases
5 blocks:      31 related cases
```

No rejection anomaly or output equality was observed.

---

## 10.6 Interpretation and limitation

Phase 2C confirmed a real raw-encoder relation, but did not establish a practical attack.

It demonstrated:

> The add-one input family creates a small but highly reproducible dependence in lane-aggregate sign and rank statistics at the raw encoder output.

It did not demonstrate:

- prediction of individual factors;
- an exact factor-stream relation by itself;
- equal Garside normal forms;
- a detectable `FOLD` output-bit bias;
- a branch, permutation, Hash256, or XOF distinguisher;
- a collision or preimage attack.

The automatic study verdict was `REVIEW` because a predeclared correlation condition was confirmed.

---

## 10.7 Phase 2C conclusion

```text
Phase 2C:
REVIEW
```

Finding disposition:

```text
F2C-001 — Add-one raw-encoder lane-aggregate dependence:
CONFIRMED

Persistence after Garside normalization:
NOT OBSERVED BY THE TESTED METRICS

Persistence after FOLD:
NOT OBSERVED BY THE TESTED METRICS

Complete-construction impact:
UNKNOWN
```

Phase 2C converted the Phase 2A encoder observation from an underpowered suspicion into a confirmed, narrower research finding. It did not yet explain the cause. That localization was addressed by Phase 2D.

---

# 11. Phase 2D — Encoder localization and block-counter aliasing

## 11.1 Objective

Phase 2D was designed to answer three questions:

1. Which source lane causes the confirmed add-one dependence?
2. At which encoder stage or block does it appear?
3. Does an ordinary add-one difference survive the complete production branch path?

The study evaluated both:

- a direct path, where one was added to the encoder mixed input;
- a production path, where one was added to the branch input before `A_PRE`.

The production path was:

```text
branch input
    -> A_PRE
    -> encoder
    -> Garside normalization
    -> FOLD
    -> direct-path addition and BRANCH-C
    -> A_POST
    -> branch output
```

---

## 11.2 Predeclared design and thresholds

```text
Seed groups:                 10
Round indices:               0, 3, 7, 11
Source lanes:                0 through 7
Samples per source lane:     80 per seed and round
Total cases:                 25,600
```

The corrected statistical families were:

- direct aggregate replication;
- direct internal-stage localization;
- source-conditioned localization;
- production encoder propagation;
- production Hamming propagation;
- final branch output bits.

Important thresholds included:

```text
correlation q-value:                     < 0.001
minimum absolute correlation:            >= 0.05
minimum excess over control:             >= 0.03
primary seed consistency:                >= 8 of 10
source-localization seed consistency:    >= 6 of 10
production Hamming q-value:              < 0.001
minimum production Hamming difference:   >= 0.005
branch-bit q-value:                      < 0.001
minimum branch-bit effect:               >= 0.01
trace mismatch limit:                    0
```

---

## 11.3 Execution provenance and integrity

```text
Run ID:                 20260711T141231Z
Profile:                standard
Started:                2026-07-11T14:12:31Z
Finished:               2026-07-11T14:23:37Z
Duration:               664.914 seconds
Completed cases:        25,600 / 25,600
Master seed:            317952932156252027433540
```

Implementation provenance:

```text
Core commit:
0f514333610ce74bace424305c586320e9775b52

Configuration SHA-256:
5c5f12b30347b151ec9b0934543ee73ffc21408fc0933a1ccf6dd0a2225cf983

Production library SHA-256:
18080dc792a85a0cde06eaa8f91b34837657c691a126746f86f402511ee4253f

Result ZIP SHA-256:
c4d47787c545366bcc5e654ce89f7a970514c936ebf02e9a9f9b7c4f7a716fea
```

Integrity and trace consistency:

```text
Manifest entries valid:  14 / 14
Failures:                 0
Warnings:                 0
Checkpoints decoded:      A and B
Encoder trace checks:     1,920
Encoder mismatches:       0
Branch trace checks:      960
Branch mismatches:        0
Git tree:                 clean
```

---

## 11.4 Automatic statistical results

The Phase 2C direct aggregate effect was reproduced:

```text
Direct aggregate tests:              64
Direct aggregate confirmations:      64
Mean confirmed correlation:          0.092795
```

The source-conditioned analysis was decisive:

```text
Source-conditioned tests:            512
Supported source-conditioned results: 64
Supported source lane:               lane 0 only
Related correlation range:           0.720403 to 0.777528
Mean related correlation:            0.750722
Mean control correlation:           -0.003389
Seed replication:                    10 / 10 for all 64
Same source/output lane:              8 findings
Different output lane:               56 findings
```

The fact that 56 findings used an output lane different from the source lane indicates a whole-block relation rather than a local output-lane effect.

The same-index internal-stage family reported:

```text
Direct internal-stage tests:         448
Confirmed same-index stage effects:  0
```

This result was initially interpreted by the automatic report as failing to identify a particular `A_ENC` subround. The post-run structural analysis below explains why the same-index comparison could not detect the actual relation.

---

## 11.5 Exact input/counter block-shift identity

The production encoder constructs block `b` by copying the eight-lane mixed input, adding the block index to lane 0, and then applying `A_ENC`.

Define:

$$
B_r(x,b)=A_{\mathrm{ENC},r}(x+b e_0),
$$

where $e_0$ denotes one in lane 0 and zero in every other lane.

For the related input $x'=x+e_0$:

$$
\begin{aligned}
B_r(x',b)
&=A_{\mathrm{ENC},r}(x+e_0+b e_0)\\
&=A_{\mathrm{ENC},r}(x+(b+1)e_0)\\
&=B_r(x,b+1).
\end{aligned}
$$

Therefore:

> Adding one to input lane 0 is exactly equivalent, at the encoder block-transform level, to advancing the block counter by one.

This is an algebraic identity created by the counter-injection rule. It is not a probabilistic effect and it is not caused by a particular S-box or MDS subround.

### Why the same-index stage tests returned zero

The stage-localization family compared:

```text
base block b
against
related block b
```

The exact identity instead relates:

```text
base block b + 1
against
related block b
```

Consequently, `0 / 448` same-index confirmations do not contradict the exact block-shift relation.

---

## 11.6 Exact shifted-factor evidence in the archived samples

When the first four blocks each supply eight accepted candidates, the base factor word has the form:

$$
W(x)=C_0\Vert C_1\Vert C_2\Vert C_3,
$$

while the lane-0 add-one word has the form:

$$
W(x+e_0)=C_1\Vert C_2\Vert C_3\Vert C_4.
$$

Under that acceptance pattern, the two 32-factor words share 24 factors exactly, shifted by eight positions.

The Phase 2D archive preserved one raw sample for every seed, round, and source-lane cell. There were 40 archived raw samples with source lane 0. Independent analysis of those samples found:

```text
related block 0 = base block 1:      40 / 40
related block 1 = base block 2:      40 / 40
related block 2 = base block 3:      40 / 40
related factors 0..23 =
base factors 8..31:                  40 / 40
```

Every comparison contained all 24 expected shared factors.

This raw-sample result is consistent with the exact identity and explains the source-conditioned correlation near `0.75`: three of four eight-factor blocks are reused.

It also explains the Phase 2C aggregate magnitude:

$$
0.75 / 8 \approx 0.09375,
$$

because Phase 2C distributed the lane-0 effect across eight possible source lanes.

### Rejection limitation

The block-transform identity holds independently of candidate acceptance. However, exact 24-factor word overlap assumes that the relevant blocks provide the expected eight accepted candidates. Rejections can move factor boundaries and must be included in any complete factor-stream theorem or attack analysis.

---

## 11.7 Ordinary production-path propagation result

For the tested production family, one was added to a branch-input lane before `A_PRE`.

Results:

```text
Production encoder correlation tests:   64
Confirmed production correlations:       0
Production Hamming tests:                44
Confirmed production Hamming effects:    0
Final branch output-bit tests:         1,952
Confirmed final branch-bit effects:       0
```

Round-level mean Hamming fractions remained close to one half:

| Round | `A_PRE` related/control | FOLD related/control | Branch related/control |
|---:|---:|---:|---:|
| 0 | 0.500224 / 0.499437 | 0.500623 / 0.499662 | 0.499824 / 0.500151 |
| 3 | 0.500159 / 0.499788 | 0.500028 / 0.500200 | 0.499963 / 0.499706 |
| 7 | 0.500328 / 0.500168 | 0.500238 / 0.500085 | 0.500384 / 0.500000 |
| 11 | 0.500402 / 0.500287 | 0.500379 / 0.499338 | 0.499792 / 0.500292 |

The automatic production-propagation verdict was:

```text
PASS
```

The correct interpretation is narrow:

> An ordinary add-one difference applied before `A_PRE` did not produce a confirmed residual correlation, related-control Hamming advantage, or final branch-bit effect within the tested sample and thresholds.

---

## 11.8 Why the production PASS does not close F2D-001

`A_PRE` is a public invertible permutation on the branch state. Therefore a chosen-input study can construct branch inputs whose post-`A_PRE` states differ by exactly $e_0$.

For an arbitrary mixed state $M$:

$$
R=A_{\mathrm{PRE}}^{-1}(M),
$$

and:

$$
R'=A_{\mathrm{PRE}}^{-1}(M+e_0).
$$

Then:

$$
A_{\mathrm{PRE}}(R')=A_{\mathrm{PRE}}(R)+e_0.
$$

Such pairs force the exact encoder relation after `A_PRE`. Phase 2D did not test this chosen-post-`A_PRE` construction. It tested only an ordinary add-one perturbation before `A_PRE`.

The next propagation study must therefore follow the exact relation through:

1. Garside normalization;
2. `FOLD`;
3. branch output before and after `A_POST`;
4. reduced-round Feistel propagation;
5. the complete permutation where computationally feasible.

---

## 11.9 Security interpretation

F2D-001 is a design-level structural relation, not an implementation error.

It demonstrates:

- data lane 0 and the block counter are injected through the same additive coordinate;
- a lane-0 increment aliases a counter increment exactly at the block-transform level;
- in the archived accepted-block examples, 24 of 32 factors were reused after an eight-position shift.

It does not yet demonstrate:

- equal Garside normal forms;
- equal `FOLD` outputs;
- a predictable complete branch output;
- a differential trail through the Feistel permutation;
- a hash or XOF collision;
- a preimage advantage;
- a practical distinguisher against the complete construction.

Nevertheless, a cryptographic encoder should not normally allow input data and an internal block counter to represent the same exact transformation. The relation requires design review before any production claim or stable-parameter release.

---

## 11.10 Phase 2D conclusion

```text
Phase 2D overall:
REVIEW

Ordinary pre-A_PRE production propagation:
PASS WITHIN THE TESTED FAMILY
```

Finding disposition:

```text
F2D-001 — Encoder lane-0 input/block-counter aliasing:
CONFIRMED

F2D-002 — Ordinary pre-A_PRE add-one propagation:
NOT OBSERVED WITHIN THE TESTED SAMPLE AND THRESHOLDS

Practical complete-construction attack:
NOT DEMONSTRATED

Encoder design action:
REVIEW REQUIRED; LIKELY REDESIGN BEFORE PRODUCTION
```

The result does not justify replacing the complete CTC-Σ design immediately. It does justify freezing production claims and prioritizing exact chosen-state propagation before selecting a counter-injection redesign.

---

# 12. Preliminary performance observation

Performance was not the primary target of Phases 0 or 1.

However, Phase 0 observed approximately:

```text
34.4 ms per forward permutation
34.8 ms per inverse permutation
```

on the tested AMD Athlon 3000G system.

This corresponds very roughly to:

```text
about 29 permutations per second
```

and suggests low throughput for the current clarity-oriented reference implementation.

This is not a formal benchmark. Phase 3 must isolate:

- ARITH;
- constant derivation;
- encoder;
- Garside normalization;
- fold;
- branch function;
- complete permutation;
- absorb and squeeze;
- Hash256 and XOF throughput.

The current implementation prioritizes reference clarity and testability over optimization.

---

# 13. Current security interpretation

The evidence currently supports the following statement:

> The evaluated CTC-Σ implementation is functionally coherent in the tested environment and exhibits strong observed diffusion and generally balanced final statistical behavior. The Phase 1B bit-159 anomaly was not reproduced. The eight-hour Phase 2B production study found rapid diffusion for the tested local families, no exact commutation for the tested global transformations, no corrected final bit effect, and no production trace mismatch. Phase 2C then confirmed a reproducible add-one dependence at the raw encoder lane-aggregate level. Phase 2D localized that dependence to source lane 0, and independent post-run analysis identified an exact identity between incrementing input lane 0 and advancing the encoder block counter. In 40 archived source-lane-0 samples, the related factor word shared 24 of 32 factors exactly after an eight-position shift. An ordinary add-one perturbation applied before `A_PRE` did not produce a confirmed downstream effect in the tested production path, but the stronger chosen-post-`A_PRE` relation has not yet been propagated. No practical cryptographic attack has been demonstrated. The encoder relation requires design review and likely redesign before production use or a stable security claim.

The current evidence must be interpreted with the following qualifications:

- Phase 1 and Phase 1B passed within their tested statistical scope, but statistical tests cannot establish collision, preimage, differential, or algebraic security;
- Phase 2B passed for the predefined local-difference and simple global-symmetry families, but average Hamming behavior does not establish formal differential bounds or exclude structured high-probability trails;
- the Phase 2B round trace is implementation-consistent but not a completely independent reimplementation;
- the Phase 2B global bit tests had strong power for large persistent effects but cannot exclude every isolated sub-percentage bias after correction across 17,568 hypotheses;
- Phase 2C confirmed 64 add-one raw-encoder aggregate correlations but found no confirmed `FOLD` bit effect;
- Phase 2D confirmed that all source-conditioned effects came from lane 0 and established the exact data/counter block-shift identity;
- the Phase 2D automatic stage family compared equal block indices and therefore could not detect the cross-block identity $B_r(x+e_0,b)=B_r(x,b+1)$;
- the Phase 2D production `PASS` concerns an ordinary add-one perturbation before `A_PRE`; it does not test chosen branch inputs that force an exact $e_0$ difference after `A_PRE`;
- the exact 24-factor shifted overlap was checked in all 40 archived source-lane-0 raw samples, while rejection-sensitive behavior over every executed case was not exported as a full factor-stream proof;
- the Phase 2A standard profile used reduced fields and small samples for several experiments;
- the production-permutation KAT was skipped in the recorded algebraic run;
- the SageMath/Gröbner experiment was skipped;
- the no-braid knockout result concerns avalanche only and does not establish component redundancy;
- no complete independent cryptanalytic evaluation has been performed;
- successful ASan and UBSan execution remains pending.

The evidence does **not** support statements such as:

- “CTC-Σ is secure.”
- “CTC-Σ provides 256-bit collision security.”
- “CTC-Σ provides 256-bit preimage security.”
- “Twelve rounds are proven sufficient.”
- “The braid layer adds a quantified security margin.”
- “The confirmed encoder relation is harmless in the complete construction.”
- “The ordinary pre-`A_PRE` propagation PASS closes the encoder finding.”
- “The research model has been fully validated against the production implementation.”
- “CTC-Σ is safe for passwords, signatures, blockchains, files, or network protocols.”

# 14. Next research actions

## Immediate

1. Preserve the Phase 1B, Phase 2B, Phase 2C, and Phase 2D result archives, research-source archives, configurations, manifests, and SHA-256 hashes together.
2. Record Phase 2C as `REVIEW`, Phase 2D as `REVIEW`, and the ordinary pre-`A_PRE` propagation sub-result as `PASS` within its tested family.
3. Execute a chosen-post-`A_PRE` propagation study that constructs $R=A_{\mathrm{PRE}}^{-1}(M)$ and $R'=A_{\mathrm{PRE}}^{-1}(M+e_0)$.
4. Track the exact cross-block relation, rejection behavior, shifted factor overlap, Garside normal forms, `FOLD`, pre- and post-`A_POST` states, and reduced-round Feistel outputs.
5. Do not change the full CTC-Σ core until the propagation impact is measured; however, treat the encoder counter-injection rule as a likely redesign target before production.
6. Execute `production_permutation_kat` against the current CTC-Σ known-answer vectors.
7. Confirm the reduced-field S-box differential spectrum with a larger field and a substantially larger random-permutation baseline.
8. Compare full and no-braid variants using attack-oriented metrics rather than ordinary avalanche alone.
9. Repair or replace the sanitizer environment and complete Phase 0F.
10. Continue using a clean source-tree commit or immutable source archive for every cryptanalytic execution.

## Exact encoder-relation propagation

The next focused study should:

1. Generate arbitrary mixed states $M$ and construct exact preimages through `A_PRE` inverse.
2. Verify, before all statistical analysis, that the two production paths reach $M$ and $M+e_0$ immediately before the encoder.
3. Compare related block `b` with base block `b+1`, not only equal block indices.
4. Record candidate-acceptance masks and rejection locations for every generated block.
5. Measure exact shifted overlap for signs, Lehmer indices, factors, and complete words.
6. Compare Garside outputs using:
   - exact normal-form equality;
   - infimum difference;
   - canonical-length difference;
   - common prefix and suffix lengths;
   - group quotient or conjugacy-related features where feasible.
7. Measure `FOLD`, direct-path, `A_POST`, branch, and reduced-round Feistel propagation.
8. Compare against independent controls and apply predeclared multiple-testing correction.
9. Preserve raw examples for both ordinary accepted-block cases and rejection-boundary cases.
10. Determine whether the shared 24-factor middle word yields a useful group-theoretic simplification or differential advantage.

## Encoder redesign criteria

A replacement counter-injection method should be evaluated only after the propagation study. Any candidate should satisfy:

- no exact equivalence between changing input data and advancing the internal block counter;
- explicit domain separation between user-derived state and block index;
- deterministic cross-platform behavior;
- preserved unbiased candidate decoding and rejection behavior;
- compatibility with inverse, reproducibility, and known-answer testing requirements;
- no reduction in measured diffusion or structural resistance;
- acceptable performance and implementation complexity.

Potential design families may include a separately derived block constant or a dedicated counter-domain transformation, but no replacement should be adopted without repeating correctness, statistical, structural, and differential tests.

After any encoder redesign, at minimum rerun:

- Phase 0 functional and reproducibility tests;
- Phase 1 diffusion tests;
- Phase 1B-style targeted bit confirmation where relevant;
- Phase 2B production differential tests;
- Phase 2C encoder-dependence tests;
- Phase 2D localization and propagation tests;
- known-answer vectors and cross-compiler evidence.

## Algebraic and differential confirmation

1. Repeat the S-box DDT experiment over $\mathbb F_{8191}$.
2. Increase the random-permutation comparison baseline substantially.
3. Propagate the highest-probability S-box differentials through:
   - one ARITH subround;
   - complete `A_PRE`;
   - complete `A_POST`;
   - the branch function;
   - reduced-round Feistel permutations.
4. Test the exact central S-box symmetry through multiple subrounds and MDS layers.
5. Execute the SageMath Gröbner experiment and record:
   - term order;
   - variable count;
   - equation count;
   - degree growth;
   - runtime;
   - peak memory;
   - solver result;
   - timeout or failure reason.
6. Extend invariant searches to:
   - eight lanes;
   - degrees 3 and 4 where feasible;
   - multiple Feistel rounds;
   - mixed arithmetic and braid-derived features.

## Braid-layer contribution

Compare full and modified constructions using metrics beyond avalanche:

- differential trail probabilities;
- integral propagation;
- algebraic-system size and solving time;
- invariant and affine-subspace searches;
- fixed points and short cycles in toy instances;
- digest-reduced collision and preimage experiments;
- encoder-to-normal-form multiplicity;
- Keep-window sizes `8, 16, 24, 32`;
- factor counts below and above 32;
- raw factors versus normalized factors;
- full fold versus simplified fold.

The braid layer should be considered cryptographically justified only after it produces a measurable improvement against one or more relevant attack classes.

## Phase 2D disposition

The direct lane-0 input/counter relation is confirmed. Therefore:

1. finding `F2D-001` remains open for complete-path impact but closed as to existence;
2. same-index stage tests must not be used to claim the relation disappears inside `A_ENC`;
3. the ordinary pre-`A_PRE` propagation result is recorded as a narrow negative result, not a closure of the chosen-state question;
4. counter-injection redesign work may be prepared in parallel, but parameter changes should wait for exact propagation evidence;
5. production-readiness language must explicitly mention the confirmed encoder relation.

## Phase 2C disposition

The statistically powered study confirmed the earlier encoder suspicion. Therefore:

1. finding `F2A-003` is superseded;
2. finding `F2C-001` is recorded as `CONFIRMED`;
3. the absence of corrected `FOLD` bit effects remains a scoped negative result;
4. future encoder analysis must use source-lane conditioning and shifted-block comparisons.

## Phase 2B disposition

The eight-hour production study passed all predeclared hypotheses.

Therefore:

1. the tested local differential families are recorded as rapidly diffusing in the production permutation;
2. the tested global negation, scaling, lane-order, lane-rotation, and half-swap transformations are recorded as non-commuting at the final permutation;
3. no final bit-bias finding is opened from this study;
4. output bit 929 under the half-swap commutation residual remains an exploratory, unconfirmed candidate only;
5. future Phase 2 work must focus on exact structural relations, differential probabilities, S-box structure, braid contribution, and algebraic attacks rather than repeat broad avalanche tests.

## Phase 1B disposition

The pre-registered bit-159 hypothesis was not confirmed. Therefore:

1. Phase 1 is closed as `PASS` within the tested statistical scope.
2. Finding `F1-001` is closed as `NOT REPRODUCED`.
3. The exploratory 40-byte and 63-byte observations remain documented but do not currently justify a new confirmatory phase.
4. The principal research effort proceeds to Phase 2 differential, structural, and algebraic cryptanalysis.
5. Formal Phase 3 performance profiling may begin in parallel.

# 15. Update procedure

When adding a new study:

1. add or update the phase status table;
2. document the objective;
3. document the exact experiment design;
4. record the sample sizes and seeds;
5. record the acceptance thresholds before interpreting results;
6. summarize the results numerically;
7. separate observations from interpretations;
8. state limitations explicitly;
9. state what the result does not prove;
10. update the next-action list;
11. add a change-log entry below.

---

# 16. Research record change log

## 2026-07-11 — Phase 2D encoder localization and structural analysis completed

Added:

- Phase 2D execution provenance, configuration, integrity, and trace-consistency results;
- reproduction of all 64 direct aggregate add-one correlations;
- source-conditioned localization to lane 0 only;
- the `0.720403`–`0.777528` source-conditioned correlation range;
- the distinction between same-index stage comparisons and the correct cross-block comparison;
- the exact identity $B_r(x+e_0,b)=B_r(x,b+1)$;
- independent verification of exact block shifting and 24-factor overlap in all 40 archived source-lane-0 raw samples;
- ordinary pre-`A_PRE` production-path results with zero confirmed encoder, Hamming, or final branch-bit effects;
- the limitation that chosen branch inputs can force the exact post-`A_PRE` relation;
- finding `F2D-001`, classified as confirmed with complete-construction impact unknown;
- finding `F2D-002`, classified as not observed within the ordinary tested production family;
- updated security interpretation, design status, and next-action priorities.

The study completed 25,600 cases with zero failures, zero warnings, zero trace mismatches, and a clean core tree. The direct effect was entirely attributable to source lane 0. The archived raw evidence and the encoder definition show that adding one to lane 0 aliases advancing the block counter by one. This is a confirmed design-level structural relation, not a demonstrated complete-construction attack.

---

## 2026-07-11 — Phase 2C encoder dependence confirmation completed

Added:

- Phase 2C objective and predeclared statistical criteria;
- execution provenance for 160,000 cases across ten seed groups, four rounds, and four related-input families;
- 3,072 encoder correlation tests and 7,808 `FOLD` bit tests;
- 64 confirmed correlations, all within the add-one family and lane-aggregate sign/rank metrics;
- correlation magnitude, control excess, and seed-replication results;
- same-position factor sensitivity, normal-form comparison, and `FOLD` Hamming results;
- zero exact encoder words, normal forms, or `FOLD` outputs;
- zero confirmed `FOLD` output-bit effects;
- finding `F2C-001`, classified as confirmed at the raw encoder aggregate level;
- the supersession of the underpowered Phase 2A encoder observation.

Phase 2C established that the earlier encoder signal was real but smaller and more specific than the initial 32-sample estimate. The study did not explain the cause and did not demonstrate persistence after Garside normalization or `FOLD`; Phase 2D subsequently localized the cause.

---

## 2026-07-11 — Phase 2B eight-hour production study completed

Added:

- Phase 2B objective, predeclared hypotheses, and thresholds;
- execution provenance for the eight-hour production run;
- result-archive, core-commit, and library hashes;
- manifest and hourly-snapshot integrity results;
- six local differential-family results across all twelve rounds;
- six global symmetry and commutation results;
- production trace verification;
- final Hamming-distribution and global output-bit analysis;
- the exploratory, non-confirmed bit-929 observation;
- the statistical-power limitation of the global bit analysis;
- the infrastructure note concerning `last_snapshot_active_second`;
- the Phase 2B `PASS` conclusion;
- the retained `REVIEW / INCOMPLETE` classification for Phase 2 overall;
- revised current-security interpretation and next actions.

The run completed 332,241 paired cases across ten seed groups and twelve predefined categories. Every local family reached the 49%–51% mean changed-bit band by round three or earlier. No tested global transformation commuted exactly with the final production permutation. No output bit survived the predeclared global FDR correction, and no trace mismatch was found in 2,584 verification states.

The result strengthens the empirical production-diffusion evidence but does not close the open Phase 2A findings or establish formal differential-security bounds.

---


## 2026-07-10 — Phase 1B confirmatory study completed

Updated:

- Phase 1B from `IN PROGRESS` to `PASS`;
- Phase 1 from `REVIEW` to `PASS within the tested statistical scope`;
- finding `F1-001` from an open bit-159 anomaly to `CLOSED — NOT REPRODUCED`;
- execution provenance for the 20,000-comparison confirmatory run;
- pooled, per-seed, confidence-interval, XOF-comparison, avalanche, and multiple-testing results;
- secondary message-length observations at 40 and 63 bytes;
- the current security interpretation;
- immediate research priorities and the Phase 1B disposition.

The completed Phase 1B study used ten independent seeds and 2,000 paired one-bit perturbations per seed. Hash256 output bit 159 changed in 9,950 of 20,000 comparisons, or 49.75%, with $p=0.483905$. The 99.9% Wilson interval included 50%, and the seed directions were not consistent. No pre-registered confirmation criterion was met.

The original Phase 1 observation remains preserved as historical evidence but is classified as a non-reproduced statistical fluctuation.

---

## 2026-07-10 — Algebraic and structural baseline

Added:

- Phase 2A execution provenance and scope;
- standard-profile experiment status table;
- exact field, Dickson, S-box, MDS, and Jacobian results;
- exact S-box central-symmetry finding;
- exhaustive reduced-field S-box differential spectrum;
- linear/quadratic invariant and structured-subspace results;
- Mersenne-related symmetry analysis;
- encoder related-input observations and statistical limitations;
- braid-relation and normal-form sensitivity results;
- component-knockout avalanche comparison;
- reduced-round no-braid avalanche results;
- polynomial-system export status;
- explicit record that production KAT and Gröbner solving were skipped;
- revised current security interpretation;
- prioritized algebraic, differential, encoder, and braid-layer follow-up actions.

The Phase 2A result was recorded as `REVIEW`, with incomplete scope, because the differential spectrum requires confirmation, two structural observations remain underpowered, and two important experiments were skipped.

---

## 2026-07-10 — Initial research record

Added:

- research vocabulary;
- Phase 0 methodology and results;
- GCC/Clang reproducibility findings;
- sanitizer limitation;
- Phase 1 diffusion and statistical findings;
- Hash256 bit 159 review item;
- braid-distribution findings;
- Phase 1B plan and confirmation criteria;
- preliminary performance observation;
- security non-claims;
- next research actions.

---

# 17. Maintainer note

This file should remain readable by:

- implementers;
- cryptographers;
- reviewers;
- statisticians;
- security engineers;
- users evaluating project maturity.

Technical precision takes priority over promotional language. Negative, inconclusive, and blocked results must remain documented.
