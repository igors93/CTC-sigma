#include "ctc_sigma/branch.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/constants.h"
#include "ctc_sigma/encoder.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/fold.h"
#include "ctc_sigma/parameters.h"

ctc_status_t ctc_branch_apply_with_normalizer(
    const uint64_t right_half[8],
    uint32_t round_index,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint64_t branch_output[8]
) {
    uint64_t mixed_input[8];
    uint64_t folded_normal_form[8];
    ctc_signed_factor_t signed_factors[CTC_FACTORS_PER_BRANCH];
    ctc_braid_normal_form_t normal_form;
    ctc_status_t status;

    if (right_half == NULL || normalizer == NULL || branch_output == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    memcpy(mixed_input, right_half, sizeof(mixed_input));
    status = ctc_arith_apply(
        "A_PRE",
        round_index,
        mixed_input,
        CTC_ARITH_PRE_SUBROUNDS
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    status = ctc_encoder_generate_factors(
        mixed_input,
        round_index,
        signed_factors,
        NULL
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memset(&normal_form, 0, sizeof(normal_form));
    status = normalizer(
        signed_factors,
        CTC_FACTORS_PER_BRANCH,
        &normal_form,
        normalizer_context
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    if (normal_form.factor_count > CTC_MAX_NORMAL_FACTORS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    status = ctc_fold_normal_form(&normal_form, round_index, folded_normal_form);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        uint64_t round_constant;
        status = ctc_constant_derive("BRANCH-C", round_index, lane, &round_constant);
        if (status != CTC_STATUS_OK) {
            return status;
        }
        branch_output[lane] = ctc_field_add(
            ctc_field_add(mixed_input[lane], folded_normal_form[lane]),
            round_constant
        );
    }

    return ctc_arith_apply(
        "A_POST",
        round_index,
        branch_output,
        CTC_ARITH_POST_SUBROUNDS
    );
}

ctc_status_t ctc_branch_apply(
    const uint64_t right_half[8],
    uint32_t round_index,
    uint64_t branch_output[8]
) {
    return ctc_branch_apply_with_normalizer(
        right_half,
        round_index,
        ctc_braid_normalize_left,
        NULL,
        branch_output
    );
}
