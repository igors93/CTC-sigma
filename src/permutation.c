#include "ctc_sigma/permutation.h"

#include <stdint.h>
#include <string.h>

#include "ctc_sigma/branch.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/validation.h"

ctc_status_t ctc_permutation_apply_with_normalizer(
    uint64_t state[16],
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context
) {
    uint64_t left[8];
    uint64_t right[8];

    if (state == NULL || normalizer == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    {
        const ctc_status_t validation_status = ctc_validate_canonical_lanes(
            state,
            CTC_STATE_LANES
        );
        if (validation_status != CTC_STATUS_OK) {
            return validation_status;
        }
    }

    memcpy(left, state, sizeof(left));
    memcpy(right, state + CTC_BRANCH_LANES, sizeof(right));

    for (uint32_t round = 0U; round < CTC_FEISTEL_ROUNDS; ++round) {
        uint64_t branch_output[8];
        uint64_t next_right[8];
        ctc_status_t status = ctc_branch_apply_with_normalizer(
            right,
            round,
            normalizer,
            normalizer_context,
            branch_output
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }

        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            next_right[lane] = ctc_field_add(left[lane], branch_output[lane]);
        }
        memcpy(left, right, sizeof(left));
        memcpy(right, next_right, sizeof(right));
    }

    memcpy(state, left, sizeof(left));
    memcpy(state + CTC_BRANCH_LANES, right, sizeof(right));
    return CTC_STATUS_OK;
}

ctc_status_t ctc_permutation_inverse_with_normalizer(
    uint64_t state[16],
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context
) {
    uint64_t left[8];
    uint64_t right[8];

    if (state == NULL || normalizer == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    {
        const ctc_status_t validation_status = ctc_validate_canonical_lanes(
            state,
            CTC_STATE_LANES
        );
        if (validation_status != CTC_STATUS_OK) {
            return validation_status;
        }
    }

    memcpy(left, state, sizeof(left));
    memcpy(right, state + CTC_BRANCH_LANES, sizeof(right));

    for (uint32_t remaining = CTC_FEISTEL_ROUNDS; remaining > 0U; --remaining) {
        const uint32_t round = remaining - 1U;
        uint64_t branch_output[8];
        uint64_t previous_left[8];
        ctc_status_t status = ctc_branch_apply_with_normalizer(
            left,
            round,
            normalizer,
            normalizer_context,
            branch_output
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }

        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            previous_left[lane] = ctc_field_sub(right[lane], branch_output[lane]);
        }
        memcpy(right, left, sizeof(right));
        memcpy(left, previous_left, sizeof(left));
    }

    memcpy(state, left, sizeof(left));
    memcpy(state + CTC_BRANCH_LANES, right, sizeof(right));
    return CTC_STATUS_OK;
}

ctc_status_t ctc_permutation_apply(uint64_t state[16]) {
    return ctc_permutation_apply_with_normalizer(
        state,
        ctc_braid_normalize_left,
        NULL
    );
}

ctc_status_t ctc_permutation_inverse(uint64_t state[16]) {
    return ctc_permutation_inverse_with_normalizer(
        state,
        ctc_braid_normalize_left,
        NULL
    );
}
