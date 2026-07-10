# CTC-Σ Research and Evaluation Record

**Document status:** Living technical record  
**Project:** CTC-Σ v0.1  
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
| Phase 1 | Statistical behavior and diffusion | REVIEW | General behavior was strong; one isolated Hash256 bit anomaly requires confirmation. |
| Phase 1B | Hash256 anomaly confirmation | IN PROGRESS | Designed to confirm or reject the isolated observation at output bit 159. |
| Phase 2 | Reduced-round cryptanalysis | NOT STARTED | Pending Phase 1B interpretation. |
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

\[
\operatorname{XOF}(M,n+k)[0:n] = \operatorname{XOF}(M,n)
\]

### Permutation reversibility

For sampled states:

\[
P_\Sigma^{-1}(P_\Sigma(X)) = X
\]

### ARITH reversibility

For all configured arithmetic labels and subround counts:

\[
\operatorname{ARITH}^{-1}(\operatorname{ARITH}(X)) = X
\]

### Lehmer rank and unrank

The complete space of `8! = 40320` permutations was checked:

\[
\operatorname{Rank}(\operatorname{Unrank}(r)) = r
\]

### Garside normalization agreement

The C normalizer and an independently written Python normalizer were compared on random signed words.

The comparison included:

- infimum;
- canonical factor count;
- factor sequence;
- projection to \(S_8\);
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

\[
50.0015\%
\]

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
- alternating zero and \(q-1\);
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

\[
49.9363\%
\]

For the reviewed run:

```text
comparisons:        384
mean changed bits:  128.03 of 256
minimum:            105
maximum:            157
standard deviation: 8.26
```

For an ideal binomial model:

\[
E[X] = 128
\]

\[
\sigma = 8
\]

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

---

## 6.11 XOF avalanche

Mean 256-bit XOF avalanche values were:

```text
Run 0: 49.9430%
Run 1: 49.9776%
Run 2: 50.0793%
```

Combined mean:

\[
50.0000\%
\]

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

\[
-12.0248
\]

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

\[
24.0891
\]

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

### Requires targeted confirmation

- Hash256 output bit 159 produced one isolated significant observation in one seed.
- The Keep window was saturated in nearly every sampled normal form.
- Sampled BIC coverage was not exhaustive.
- Statistical testing used one machine and one principal build.

### Phase result

```text
Phase 1: REVIEW
```

The reason is narrow and explicit: Hash256 bit 159 requires a higher-powered confirmation study.

---

# 7. Phase 1B — Hash256 bit-anomaly confirmation

## 7.1 Objective

Phase 1B is a focused confirmatory study designed to determine whether the Phase 1 observation at Hash256 output bit 159 is:

1. a reproducible output bias;
2. a bias restricted to certain message lengths or input-bit locations;
3. a Hash256-domain effect not present in XOF;
4. or an ordinary statistical fluctuation.

---

## 7.2 Planned standard experiment

The standard Phase 1B profile is configured for:

```text
10 independent seeds
2,000 one-bit message perturbations per seed
20,000 total comparisons
```

For every comparison, the study computes:

```text
Hash256(original message)
Hash256(one-bit-modified message)
XOF-256(original message)
XOF-256(one-bit-modified message)
```

The study records:

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

Boundary-focused lengths include:

```text
39, 40, 41
79, 80, 81
```

These lengths test behavior around the 40-byte rate boundary.

---

## 7.3 Confirmation criteria

The bit-159 observation will be classified as confirmed only if all of the following hold:

1. the combined two-sided test has:

\[
p < 0.001
\]

2. the 99.9% Wilson confidence interval excludes 50%;

3. the deviation has the same direction in at least 7 of 10 independent seeds.

Additional evidence will be considered stronger if the effect:

- repeats at the same message lengths;
- repeats for the same relative input positions;
- remains significant after global false discovery rate correction;
- appears in Hash256 but not XOF under matched inputs;
- remains visible after excluding individual seed groups.

---

## 7.4 Interpretation rules

### If the criteria are not met

The Phase 1 observation should be reclassified as:

```text
Statistical fluctuation not reproduced by the confirmatory study
```

Phase 1 may then be closed as `PASS`, while preserving the historical anomaly record.

### If only weak evidence remains

The result remains:

```text
REVIEW
```

A larger or differently stratified study will be required.

### If all confirmation criteria are met

The result becomes:

```text
CONFIRMED STATISTICAL ANOMALY
```

This would not by itself prove a practical attack, but it would require:

- root-cause analysis;
- round-level localization;
- domain-separation analysis;
- length-class analysis;
- review of padding and squeeze behavior;
- possible specification revision.

---

## 7.5 Current status

```text
Phase 1B: IN PROGRESS
```

No Phase 1B conclusion is recorded yet.

The result archive must be analyzed before this section is updated.

---

# 8. Preliminary performance observation

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

# 9. Current security interpretation

The evidence currently supports the following statement:

> The evaluated CTC-Σ v0.1 implementation is functionally coherent in the tested environment and exhibits strong observed diffusion and generally balanced statistical behavior. No practical cryptographic attack has been demonstrated. One isolated Hash256 output-bit anomaly remains under confirmatory analysis. These results are insufficient to claim cryptographic security or production readiness.

The evidence does **not** support statements such as:

- “CTC-Σ is secure.”
- “CTC-Σ provides 256-bit collision security.”
- “Twelve rounds are proven sufficient.”
- “The braid layer adds a quantified security margin.”
- “CTC-Σ is safe for passwords, signatures, blockchains, files, or network protocols.”

---

# 10. Next research actions

## Immediate

1. Complete Phase 1B.
2. Update this document with the Phase 1B result.
3. Repair or replace the sanitizer environment and complete Phase 0F.
4. Preserve the final result archives and their SHA-256 hashes.

## After Phase 1B

If bit 159 is not confirmed:

1. close Phase 1 as `PASS`;
2. begin reduced-round Phase 2;
3. begin formal Phase 3 profiling.

If bit 159 is confirmed:

1. pause claims of statistical adequacy;
2. localize the earliest round where the bias appears;
3. compare Hash256 and XOF initialization;
4. test only message lengths associated with the effect;
5. analyze padding boundaries;
6. test modified or removed components;
7. decide whether the design or implementation requires correction.

## Phase 2 priorities

- reduced rounds from 1 through 12;
- differential propagation;
- fixed points and short cycles in toy instances;
- invariant and symmetric-state searches;
- digest-reduced collision and preimage experiments;
- knockout experiments;
- Keep-window sizes `8, 16, 24, 32`;
- factor counts below and above 32;
- comparison of Drop/Keep handling.

---

# 11. Update procedure

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

# 12. Research record change log

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

## 13. Maintainer note

This file should remain readable by:

- implementers;
- cryptographers;
- reviewers;
- statisticians;
- security engineers;
- users evaluating project maturity.

Technical precision takes priority over promotional language. Negative, inconclusive, and blocked results must remain documented.
