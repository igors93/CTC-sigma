#ifndef CTC_SIGMA_DYNAMIC_ARITH_H
#define CTC_SIGMA_DYNAMIC_ARITH_H

#include <stdint.h>

#include "ctc_sigma/braid_descriptor.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ctc_dynamic_arith_config {
    uint64_t round_constants[CTC_ARITH_DYNAMIC_SUBROUNDS][8];
    uint64_t sbox_a[CTC_ARITH_DYNAMIC_SUBROUNDS][8];
    uint64_t sbox_b[CTC_ARITH_DYNAMIC_SUBROUNDS][8];
    uint64_t sbox_c[CTC_ARITH_DYNAMIC_SUBROUNDS][8];
    uint8_t lane_permutation[8];
    uint8_t degrees[CTC_ARITH_DYNAMIC_SUBROUNDS];
} ctc_dynamic_arith_config_t;

ctc_status_t ctc_dynamic_arith_config_build(
    const ctc_braid_descriptor_t *descriptor,
    uint32_t round_index,
    ctc_dynamic_arith_config_t *config_out
);

ctc_status_t ctc_dynamic_arith_inject_descriptor(
    const uint64_t state[8],
    const ctc_braid_descriptor_t *descriptor,
    uint64_t output[8]
);

ctc_status_t ctc_dynamic_arith_apply(
    uint64_t state[8],
    const ctc_dynamic_arith_config_t *config
);

#ifdef __cplusplus
}
#endif

#endif
