#ifndef CTC_SIGMA_BRAID_H
#define CTC_SIGMA_BRAID_H

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/encoder.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ctc_braid_normal_form {
    int64_t infimum;
    size_t factor_count;
    uint16_t factors[CTC_MAX_NORMAL_FACTORS];
} ctc_braid_normal_form_t;

typedef ctc_status_t (*ctc_braid_normalizer_fn)(
    const ctc_signed_factor_t *word,
    size_t word_length,
    ctc_braid_normal_form_t *normal_form_out,
    void *user_context
);

ctc_status_t ctc_braid_simple_permutation(
    uint16_t simple_index,
    uint8_t permutation_out[8]
);

/*
 * Validate the canonical representation NF(W) = Delta^p * x_1 * ... * x_m.
 * Every stored factor must be proper (Lehmer ranks 1..40318), and every
 * adjacent pair must satisfy the left-weighted condition. This validates the
 * representation itself; it cannot prove that an injected normalizer
 * preserved the input braid word.
 */
ctc_status_t ctc_braid_validate_normal_form(
    const ctc_braid_normal_form_t *normal_form
);

/*
 * Exact left Garside normal form for a word of signed simple factors in B_8:
 * NF(W) = Delta^infimum * x_1 * ... * x_m with proper left-weighted factors.
 * The permutation-braid convention is documented in src/braid.c. The function
 * matches ctc_braid_normalizer_fn so independent implementations can be
 * injected for cross-validation; user_context is unused and may be NULL.
 */
ctc_status_t ctc_braid_normalize_left(
    const ctc_signed_factor_t *word,
    size_t word_length,
    ctc_braid_normal_form_t *normal_form_out,
    void *user_context
);

#ifdef __cplusplus
}
#endif

#endif
