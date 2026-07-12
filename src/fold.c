#include "ctc_sigma/fold.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/constants.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/parameters.h"

#define CTC_TOKEN_VERSION UINT64_C(0x01)
#define CTC_TOKEN_ROUND UINT64_C(0x02)
#define CTC_TOKEN_INFIMUM UINT64_C(0x03)
#define CTC_TOKEN_LENGTH UINT64_C(0x04)
#define CTC_TOKEN_PREFIX_LENGTH UINT64_C(0x05)
#define CTC_TOKEN_DROP_FACTOR UINT64_C(0x10)
#define CTC_TOKEN_KEEP_FACTOR UINT64_C(0x11)
#define CTC_TOKEN_COUNT UINT64_C(0xFE)
#define CTC_TOKEN_END UINT64_C(0xFF)

static uint64_t ctc_fold_token(uint64_t tag, uint64_t value) {
    return (tag << 48U) + value;
}

static ctc_status_t ctc_zigzag_encode(int64_t value, uint64_t *encoded_out) {
    if (encoded_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (value == INT64_MIN) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    *encoded_out = value >= 0
        ? (uint64_t)value * UINT64_C(2)
        : (uint64_t)(-value) * UINT64_C(2) - UINT64_C(1);
    if (*encoded_out >= (UINT64_C(1) << 48U)) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    return CTC_STATUS_OK;
}

static ctc_status_t ctc_append_token(
    ctc_fold_tokens_t *tokens,
    uint64_t tag,
    uint64_t value
) {
    if (tokens->token_count >= CTC_MAX_FOLD_TOKENS || value >= (UINT64_C(1) << 48U)) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    tokens->values[tokens->token_count] = ctc_fold_token(tag, value);
    ++tokens->token_count;
    return CTC_STATUS_OK;
}

ctc_status_t ctc_fold_tokenize_normal_form(
    const ctc_braid_normal_form_t *normal_form,
    uint32_t round_index,
    ctc_fold_tokens_t *tokens_out
) {
    ctc_status_t status;
    uint64_t encoded_infimum;
    size_t prefix_count;

    if (normal_form == NULL || tokens_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    status = ctc_braid_validate_normal_form(normal_form);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memset(tokens_out, 0, sizeof(*tokens_out));
    prefix_count = normal_form->factor_count > CTC_GARSIDE_KEEP_WINDOW
        ? normal_form->factor_count - CTC_GARSIDE_KEEP_WINDOW
        : 0U;
    tokens_out->prefix_factor_count = prefix_count;

    status = ctc_zigzag_encode(normal_form->infimum, &encoded_infimum);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    status = ctc_append_token(tokens_out, CTC_TOKEN_VERSION, 3U);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_append_token(tokens_out, CTC_TOKEN_ROUND, round_index);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_append_token(tokens_out, CTC_TOKEN_INFIMUM, encoded_infimum);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_append_token(tokens_out, CTC_TOKEN_LENGTH, normal_form->factor_count);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_append_token(tokens_out, CTC_TOKEN_PREFIX_LENGTH, prefix_count);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (size_t index = 0U; index < normal_form->factor_count; ++index) {
        const uint16_t factor = normal_form->factors[index];
        status = ctc_append_token(
            tokens_out,
            index < prefix_count ? CTC_TOKEN_DROP_FACTOR : CTC_TOKEN_KEEP_FACTOR,
            factor
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
    }

    return CTC_STATUS_OK;
}

static ctc_status_t ctc_fold_add_domain_constants(
    uint64_t state[8],
    uint32_t round_index,
    uint32_t group_index
) {
    for (uint32_t lane = 5U; lane < CTC_BRANCH_LANES; ++lane) {
        uint64_t constant;
        const uint32_t second_index = (group_index << 8U) | lane;
        const ctc_status_t status = ctc_constant_derive(
            "FOLD-GROUP",
            round_index,
            second_index,
            &constant
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        state[lane] = ctc_field_add(state[lane], constant);
    }
    return CTC_STATUS_OK;
}

ctc_status_t ctc_fold_normal_form(
    const ctc_braid_normal_form_t *normal_form,
    uint32_t round_index,
    uint64_t folded_state_out[8]
) {
    ctc_fold_tokens_t tokens;
    ctc_status_t status;
    size_t offset = 0U;
    uint32_t group_index = 0U;

    if (normal_form == NULL || folded_state_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    status = ctc_fold_tokenize_normal_form(normal_form, round_index, &tokens);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        status = ctc_constant_derive("FOLD-IV", round_index, lane, &folded_state_out[lane]);
        if (status != CTC_STATUS_OK) {
            return status;
        }
    }

    while (offset < tokens.token_count) {
        const size_t remaining = tokens.token_count - offset;
        const size_t used = remaining < 4U ? remaining : 4U;

        for (size_t token_index = 0U; token_index < 4U; ++token_index) {
            const uint64_t token = token_index < used
                ? tokens.values[offset + token_index]
                : 0U;
            folded_state_out[token_index] = ctc_field_add(
                folded_state_out[token_index],
                token
            );
        }
        folded_state_out[4] = ctc_field_add(folded_state_out[4], group_index);
        if (used < 4U) {
            folded_state_out[4] = ctc_field_add(folded_state_out[4], used);
        }
        status = ctc_fold_add_domain_constants(folded_state_out, round_index, group_index);
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_arith_apply(
            "A_FOLD",
            round_index,
            folded_state_out,
            CTC_ARITH_FOLD_GROUP_SUBROUNDS
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }

        offset += used;
        ++group_index;
    }

    folded_state_out[0] = ctc_field_add(
        folded_state_out[0],
        ctc_fold_token(CTC_TOKEN_COUNT, tokens.token_count + 2U)
    );
    folded_state_out[1] = ctc_field_add(
        folded_state_out[1],
        ctc_fold_token(CTC_TOKEN_END, 0U)
    );
    folded_state_out[4] = ctc_field_add(folded_state_out[4], group_index);
    status = ctc_fold_add_domain_constants(folded_state_out, round_index, group_index);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_arith_apply(
        "A_FOLD",
        round_index,
        folded_state_out,
        CTC_ARITH_FOLD_FINAL_SUBROUNDS
    );
}
