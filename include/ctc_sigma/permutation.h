#ifndef CTC_SIGMA_PERMUTATION_H
#define CTC_SIGMA_PERMUTATION_H

#include <stdint.h>

#include "ctc_sigma/braid.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

ctc_status_t ctc_permutation_apply(uint64_t state[16]);
ctc_status_t ctc_permutation_inverse(uint64_t state[16]);

ctc_status_t ctc_permutation_apply_with_normalizer(
    uint64_t state[16],
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context
);

ctc_status_t ctc_permutation_inverse_with_normalizer(
    uint64_t state[16],
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context
);

#ifdef __cplusplus
}
#endif

#endif
