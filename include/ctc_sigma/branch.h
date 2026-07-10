#ifndef CTC_SIGMA_BRANCH_H
#define CTC_SIGMA_BRANCH_H

#include <stdint.h>

#include "ctc_sigma/braid.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
