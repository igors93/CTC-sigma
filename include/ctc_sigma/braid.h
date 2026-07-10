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
 * The exact left Garside normalizer is intentionally isolated behind this API.
 * The first implementation milestone leaves this function explicit rather than
 * silently substituting a non-equivalent braid reduction.
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
