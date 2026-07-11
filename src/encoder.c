#include "ctc_sigma/encoder.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/parameters.h"

ctc_status_t ctc_encoder_decode_candidate(
    uint64_t lane_value,
    ctc_signed_factor_t *factor_out,
    ctc_encoder_candidate_result_t *result_out
) {
    const uint64_t two_to_32 = UINT64_C(1) << 32U;
    const uint64_t threshold_32 =
        (CTC_FIELD_MODULUS / two_to_32) * two_to_32;
    const uint64_t two_to_31 = UINT64_C(1) << 31U;
    const uint64_t simple_threshold =
        (two_to_31 / CTC_NON_IDENTITY_FACTOR_COUNT)
        * CTC_NON_IDENTITY_FACTOR_COUNT;
    uint64_t word;
    uint64_t factor_value;

    if (factor_out == NULL || result_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (lane_value >= CTC_FIELD_MODULUS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    memset(factor_out, 0, sizeof(*factor_out));
    if (lane_value >= threshold_32) {
        *result_out = CTC_ENCODER_CANDIDATE_REJECT_FIELD_WORD;
        return CTC_STATUS_OK;
    }

    word = lane_value & UINT64_C(0xFFFFFFFF);
    factor_value = word & UINT64_C(0x7FFFFFFF);
    if (factor_value >= simple_threshold) {
        *result_out = CTC_ENCODER_CANDIDATE_REJECT_SIMPLE_INDEX;
        return CTC_STATUS_OK;
    }

    factor_out->sign =
        (word & (UINT64_C(1) << 31U)) == 0U ? INT8_C(1) : INT8_C(-1);
    factor_out->simple_index = (uint16_t)(
        1U + (factor_value % CTC_NON_IDENTITY_FACTOR_COUNT)
    );
    *result_out = CTC_ENCODER_CANDIDATE_ACCEPTED;
    return CTC_STATUS_OK;
}

ctc_status_t ctc_encoder_generate_block(
    const uint64_t mixed_input[8],
    uint32_t round_index,
    uint32_t block_index,
    uint64_t block_out[8]
) {
    uint64_t block[CTC_BRANCH_LANES];
    ctc_status_t status;

    if (mixed_input == NULL || block_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS
        || block_index >= CTC_ENCODER_MAX_BLOCKS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    /*
     * CTC-Sigma v0.2 domain-separates the block in A_ENC's constants. The
     * data vector must remain byte-for-byte unchanged before the first
     * nonlinear subround, otherwise input/counter aliasing can reappear.
     */
    memcpy(block, mixed_input, sizeof(block));
    status = ctc_arith_apply_encoder(
        round_index,
        block_index,
        block,
        CTC_ARITH_ENCODER_SUBROUNDS
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memcpy(block_out, block, sizeof(block));
    return CTC_STATUS_OK;
}

ctc_status_t ctc_encoder_generate_factors(
    const uint64_t mixed_input[8],
    uint32_t round_index,
    ctc_signed_factor_t factors_out[CTC_FACTORS_PER_BRANCH],
    size_t *generated_block_count_out
) {
    size_t generated = 0U;

    if (mixed_input == NULL || factors_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    memset(factors_out, 0, sizeof(*factors_out) * CTC_FACTORS_PER_BRANCH);
    if (generated_block_count_out != NULL) {
        *generated_block_count_out = 0U;
    }

    for (uint32_t block_index = 0U;
         block_index < CTC_ENCODER_MAX_BLOCKS && generated < CTC_FACTORS_PER_BRANCH;
         ++block_index) {
        uint64_t block[CTC_BRANCH_LANES];
        ctc_status_t status = ctc_encoder_generate_block(
            mixed_input,
            round_index,
            block_index,
            block
        );

        if (status != CTC_STATUS_OK) {
            return status;
        }

        for (uint32_t lane = 0U;
             lane < CTC_BRANCH_LANES && generated < CTC_FACTORS_PER_BRANCH;
             ++lane) {
            ctc_signed_factor_t candidate;
            ctc_encoder_candidate_result_t candidate_result;

            status = ctc_encoder_decode_candidate(
                block[lane],
                &candidate,
                &candidate_result
            );
            if (status != CTC_STATUS_OK) {
                return status;
            }
            if (candidate_result != CTC_ENCODER_CANDIDATE_ACCEPTED) {
                continue;
            }

            factors_out[generated] = candidate;
            ++generated;
        }

        if (generated_block_count_out != NULL) {
            *generated_block_count_out = (size_t)block_index + 1U;
        }
    }

    return generated == CTC_FACTORS_PER_BRANCH
        ? CTC_STATUS_OK
        : CTC_STATUS_REJECTION_LIMIT;
}
