#include "ctc_sigma/braid_descriptor.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/constants.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/fold.h"
#include "ctc_sigma/parameters.h"

ctc_status_t ctc_braid_descriptor_build(
    const ctc_braid_normal_form_t *normal_form,
    uint32_t round_index,
    ctc_braid_descriptor_t *descriptor_out
) {
    ctc_fold_tokens_t tokens;
    uint64_t folded[CTC_BRANCH_LANES];
    uint64_t working[CTC_BRANCH_LANES];
    uint64_t domain[CTC_BRANCH_LANES];
    ctc_status_t status;

    if (normal_form == NULL || descriptor_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    status = ctc_braid_validate_normal_form(normal_form);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_fold_tokenize_normal_form(normal_form, round_index, &tokens);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_fold_normal_form(normal_form, round_index, folded);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_v03_constant_derive_lanes(
        "BRAID-DESCRIPTOR",
        CTC_V03_CONSTANT_DESCRIPTOR_IV,
        round_index,
        0U,
        0U,
        domain
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        const uint32_t next = (lane + 3U) % CTC_BRANCH_LANES;
        const uint32_t far = (lane + 5U) % CTC_BRANCH_LANES;
        working[lane] = ctc_field_add(
            ctc_field_add(folded[lane], domain[lane]),
            ctc_field_add(
                ctc_field_mul((uint64_t)(lane + 2U), folded[next]),
                ctc_field_mul(folded[far], folded[far])
            )
        );
    }
    status = ctc_arith_apply(
        "BRAID-DESCRIPTOR-FINAL",
        round_index,
        working,
        2U
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memset(descriptor_out, 0, sizeof(*descriptor_out));
    memcpy(descriptor_out->lanes, working, sizeof(working));
    descriptor_out->token_count = tokens.token_count;
    descriptor_out->prefix_factor_count = tokens.prefix_factor_count;
    return CTC_STATUS_OK;
}

ctc_status_t ctc_braid_descriptor_build_fixed(
    uint32_t round_index,
    ctc_braid_descriptor_t *descriptor_out
) {
    uint64_t lanes[CTC_BRANCH_LANES];
    ctc_status_t status;

    if (descriptor_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    status = ctc_v03_constant_derive_lanes(
        "FIXED-DESCRIPTOR-CONTROL",
        CTC_V03_CONSTANT_DESCRIPTOR_IV,
        round_index,
        0U,
        0U,
        lanes
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memset(descriptor_out, 0, sizeof(*descriptor_out));
    memcpy(descriptor_out->lanes, lanes, sizeof(lanes));
    return CTC_STATUS_OK;
}
