#include "ctc_sigma/encoder.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/parameters.h"

ctc_status_t ctc_encoder_generate_factors(
    const uint64_t mixed_input[8],
    uint32_t round_index,
    ctc_signed_factor_t factors_out[CTC_FACTORS_PER_BRANCH],
    size_t *generated_block_count_out
) {
    const uint64_t two_to_32 = UINT64_C(1) << 32U;
    const uint64_t threshold_32 =
        (CTC_FIELD_MODULUS / two_to_32) * two_to_32;
    const uint64_t two_to_31 = UINT64_C(1) << 31U;
    const uint64_t simple_threshold =
        (two_to_31 / CTC_NON_IDENTITY_FACTOR_COUNT)
        * CTC_NON_IDENTITY_FACTOR_COUNT;
    size_t generated = 0U;

    if (mixed_input == NULL || factors_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    for (uint32_t block_index = 0U;
         block_index < CTC_ENCODER_MAX_BLOCKS && generated < CTC_FACTORS_PER_BRANCH;
         ++block_index) {
        uint64_t block[8];
        ctc_status_t status;

        memcpy(block, mixed_input, sizeof(block));
        block[0] = ctc_field_add(block[0], (uint64_t)block_index);
        status = ctc_arith_apply(
            "A_ENC",
            round_index,
            block,
            CTC_ARITH_ENCODER_SUBROUNDS
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }

        for (uint32_t lane = 0U;
             lane < CTC_BRANCH_LANES && generated < CTC_FACTORS_PER_BRANCH;
             ++lane) {
            uint64_t word;
            uint64_t factor_value;

            if (block[lane] >= threshold_32) {
                continue;
            }
            word = block[lane] & UINT64_C(0xFFFFFFFF);
            factor_value = word & UINT64_C(0x7FFFFFFF);
            if (factor_value >= simple_threshold) {
                continue;
            }

            factors_out[generated].sign =
                (word & (UINT64_C(1) << 31U)) == 0U ? INT8_C(1) : INT8_C(-1);
            factors_out[generated].simple_index = (uint16_t)(
                1U + (factor_value % CTC_NON_IDENTITY_FACTOR_COUNT)
            );
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
