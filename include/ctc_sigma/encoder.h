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

/*
 * Classification produced when a single field lane is decoded by the exact
 * rejection rules from CTC-Sigma v0.1, Section 7.2.
 */
typedef enum ctc_encoder_candidate_result {
    CTC_ENCODER_CANDIDATE_ACCEPTED = 0,
    CTC_ENCODER_CANDIDATE_REJECT_FIELD_WORD = 1,
    CTC_ENCODER_CANDIDATE_REJECT_SIMPLE_INDEX = 2
} ctc_encoder_candidate_result_t;

/*
 * Decode one canonical F_q lane using the same branch logic used by the
 * encoder. This narrow public helper exists so boundary behavior can be
 * tested directly without relying on rare random rejections.
 *
 * On rejection, factor_out is cleared and result_out identifies the reason.
 */
ctc_status_t ctc_encoder_decode_candidate(
    uint64_t lane_value,
    ctc_signed_factor_t *factor_out,
    ctc_encoder_candidate_result_t *result_out
);

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
