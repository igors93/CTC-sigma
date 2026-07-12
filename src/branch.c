#include "ctc_sigma/branch.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/braid_descriptor.h"
#include "ctc_sigma/dynamic_arith.h"
#include "ctc_sigma/encoder.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/round_schedule.h"
#include "ctc_sigma/validation.h"

static ctc_status_t ctc_branch_apply_bridge(
    const uint64_t right_half[8],
    uint32_t round_index,
    uint64_t branch_output[8]
) {
    uint64_t working[CTC_BRANCH_LANES];
    ctc_status_t status;

    memcpy(working, right_half, sizeof(working));
    status = ctc_arith_apply(
        "V03-BRIDGE",
        round_index,
        working,
        CTC_ARITH_BRIDGE_SUBROUNDS
    );
    if (status == CTC_STATUS_OK) {
        memcpy(branch_output, working, sizeof(working));
    }
    return status;
}

static ctc_status_t ctc_branch_build_normal_form(
    const uint64_t pre_state[8],
    uint32_t round_index,
    ctc_branch_variant_t variant,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    ctc_braid_normal_form_t *normal_form_out
) {
    ctc_signed_factor_t factors[CTC_FACTORS_PER_BRANCH];
    ctc_status_t status;

    if (normal_form_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_encoder_generate_factors(
        pre_state,
        round_index,
        factors,
        NULL
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memset(normal_form_out, 0, sizeof(*normal_form_out));
    if (variant == CTC_BRANCH_VARIANT_EMPTY_NORMAL_FORM) {
        return CTC_STATUS_OK;
    }
    if (normalizer == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = normalizer(
        factors,
        CTC_FACTORS_PER_BRANCH,
        normal_form_out,
        normalizer_context
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_braid_validate_normal_form(normal_form_out);
}

static ctc_status_t ctc_branch_apply_braid(
    const uint64_t right_half[8],
    uint32_t round_index,
    ctc_branch_variant_t variant,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint64_t branch_output[8]
) {
    uint64_t pre_state[CTC_BRANCH_LANES];
    uint64_t working[CTC_BRANCH_LANES];
    ctc_braid_normal_form_t normal_form;
    ctc_braid_descriptor_t descriptor;
    ctc_dynamic_arith_config_t dynamic_config;
    ctc_status_t status;

    memcpy(pre_state, right_half, sizeof(pre_state));
    status = ctc_arith_apply(
        "V03-A-PRE",
        round_index,
        pre_state,
        CTC_ARITH_PRE_SUBROUNDS
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    if (variant == CTC_BRANCH_VARIANT_ARITHMETIC_ONLY) {
        memcpy(working, pre_state, sizeof(working));
        status = ctc_arith_apply(
            "V03-ARITHMETIC-ONLY",
            round_index,
            working,
            CTC_ARITH_DYNAMIC_SUBROUNDS
        );
        if (status == CTC_STATUS_OK) {
            memcpy(branch_output, working, sizeof(working));
        }
        return status;
    }

    if (variant == CTC_BRANCH_VARIANT_FIXED_DESCRIPTOR) {
        status = ctc_braid_descriptor_build_fixed(round_index, &descriptor);
    } else {
        status = ctc_branch_build_normal_form(
            pre_state,
            round_index,
            variant,
            normalizer,
            normalizer_context,
            &normal_form
        );
        if (status == CTC_STATUS_OK) {
            status = ctc_braid_descriptor_build(
                &normal_form,
                round_index,
                &descriptor
            );
        }
    }
    if (status != CTC_STATUS_OK) {
        return status;
    }

    status = ctc_dynamic_arith_inject_descriptor(
        pre_state,
        &descriptor,
        working
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    if (variant == CTC_BRANCH_VARIANT_NO_DYNAMIC_CONTROL) {
        status = ctc_arith_apply(
            "V03-FIXED-POST-CONTROL",
            round_index,
            working,
            CTC_ARITH_DYNAMIC_SUBROUNDS
        );
    } else {
        status = ctc_dynamic_arith_config_build(
            &descriptor,
            round_index,
            &dynamic_config
        );
        if (status == CTC_STATUS_OK) {
            status = ctc_dynamic_arith_apply(working, &dynamic_config);
        }
    }
    if (status == CTC_STATUS_OK) {
        memcpy(branch_output, working, sizeof(working));
    }
    return status;
}

ctc_status_t ctc_branch_apply_variant(
    const uint64_t right_half[8],
    uint32_t round_index,
    ctc_branch_variant_t variant,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint64_t branch_output[8]
) {
    ctc_round_type_t round_type;
    ctc_status_t status;

    if (right_half == NULL || branch_output == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS
        || (uint32_t)variant > (uint32_t)CTC_BRANCH_VARIANT_ARITHMETIC_ONLY) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    status = ctc_validate_canonical_lanes(right_half, CTC_BRANCH_LANES);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_round_schedule_get(round_index, &round_type);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    if (round_type == CTC_ROUND_ARITHMETIC_BRIDGE) {
        return ctc_branch_apply_bridge(right_half, round_index, branch_output);
    }
    return ctc_branch_apply_braid(
        right_half,
        round_index,
        variant,
        normalizer,
        normalizer_context,
        branch_output
    );
}

ctc_status_t ctc_branch_apply_with_normalizer(
    const uint64_t right_half[8],
    uint32_t round_index,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint64_t branch_output[8]
) {
    return ctc_branch_apply_variant(
        right_half,
        round_index,
        CTC_BRANCH_VARIANT_FULL,
        normalizer,
        normalizer_context,
        branch_output
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
