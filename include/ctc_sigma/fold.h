#ifndef CTC_SIGMA_FOLD_H
#define CTC_SIGMA_FOLD_H

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/braid.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ctc_fold_tokens {
    size_t prefix_factor_count;
    size_t token_count;
    uint64_t values[CTC_MAX_FOLD_TOKENS];
} ctc_fold_tokens_t;

ctc_status_t ctc_fold_tokenize_normal_form(
    const ctc_braid_normal_form_t *normal_form,
    uint32_t round_index,
    ctc_fold_tokens_t *tokens_out
);

ctc_status_t ctc_fold_normal_form(
    const ctc_braid_normal_form_t *normal_form,
    uint32_t round_index,
    uint64_t folded_state_out[8]
);

#ifdef __cplusplus
}
#endif

#endif
