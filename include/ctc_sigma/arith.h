#ifndef CTC_SIGMA_ARITH_H
#define CTC_SIGMA_ARITH_H

#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

ctc_status_t ctc_sbox_apply(
    const char *label,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t value,
    uint64_t *result_out
);

ctc_status_t ctc_sbox_inverse(
    const char *label,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t value,
    uint64_t *result_out
);

ctc_status_t ctc_mds_matrix(uint64_t matrix_out[8][8]);
ctc_status_t ctc_mds_inverse(uint64_t matrix_out[8][8]);

ctc_status_t ctc_arith_apply(
    const char *label,
    uint32_t round_index,
    uint64_t state[8],
    uint32_t subround_count
);

ctc_status_t ctc_arith_inverse(
    const char *label,
    uint32_t round_index,
    uint64_t state[8],
    uint32_t subround_count
);

/*
 * Apply the CTC-Sigma v0.2 tweakable encoder permutation A_ENC_{i,h}.
 * The block index selects the constant family and is never injected into the
 * state. subround_count may be used for trace tests but cannot exceed the
 * four normative encoder subrounds.
 */
ctc_status_t ctc_arith_apply_encoder(
    uint32_t round_index,
    uint32_t block_index,
    uint64_t state[8],
    uint32_t subround_count
);

ctc_status_t ctc_arith_inverse_encoder(
    uint32_t round_index,
    uint32_t block_index,
    uint64_t state[8],
    uint32_t subround_count
);

#ifdef __cplusplus
}
#endif

#endif
