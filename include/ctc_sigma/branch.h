#ifndef CTC_SIGMA_BRANCH_H
#define CTC_SIGMA_BRANCH_H

#include <stdint.h>

#include "ctc_sigma/braid.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ctc_branch_variant {
    CTC_BRANCH_VARIANT_FULL = 0,
    CTC_BRANCH_VARIANT_EMPTY_NORMAL_FORM = 1,
    CTC_BRANCH_VARIANT_FIXED_DESCRIPTOR = 2,
    CTC_BRANCH_VARIANT_NO_DYNAMIC_CONTROL = 3,
    CTC_BRANCH_VARIANT_ARITHMETIC_ONLY = 4
} ctc_branch_variant_t;

ctc_status_t ctc_branch_apply(
    const uint64_t right_half[8],
    uint32_t round_index,
    uint64_t branch_output[8]
);

ctc_status_t ctc_branch_apply_with_normalizer(
    const uint64_t right_half[8],
    uint32_t round_index,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint64_t branch_output[8]
);

/* Research-only controlled variants used by ablation experiments. */
ctc_status_t ctc_branch_apply_variant(
    const uint64_t right_half[8],
    uint32_t round_index,
    ctc_branch_variant_t variant,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint64_t branch_output[8]
);

#ifdef __cplusplus
}
#endif

#endif
