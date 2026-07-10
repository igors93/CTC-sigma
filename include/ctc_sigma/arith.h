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

#ifdef __cplusplus
}
#endif

#endif
