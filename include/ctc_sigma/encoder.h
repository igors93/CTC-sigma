#ifndef CTC_SIGMA_ENCODER_H
#define CTC_SIGMA_ENCODER_H

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/parameters.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ctc_signed_factor {
    uint16_t simple_index;
    int8_t sign;
} ctc_signed_factor_t;

ctc_status_t ctc_encoder_generate_factors(
    const uint64_t mixed_input[8],
    uint32_t round_index,
    ctc_signed_factor_t factors_out[CTC_FACTORS_PER_BRANCH],
    size_t *generated_block_count_out
);

#ifdef __cplusplus
}
#endif

#endif
