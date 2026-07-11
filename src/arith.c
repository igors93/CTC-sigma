#include "ctc_sigma/arith.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ctc_sigma/constants.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/parameters.h"

#if !defined(__SIZEOF_INT128__)
#error "This initial CTC-Sigma implementation requires unsigned __int128 support."
#endif

__extension__ typedef unsigned __int128 ctc_uint128_t;
__extension__ typedef __int128 ctc_int128_t;

typedef struct ctc_arith_lane_constants {
    uint64_t round_constant;
    uint64_t sbox_a;
    uint64_t sbox_b;
    uint64_t sbox_c;
} ctc_arith_lane_constants_t;

typedef ctc_status_t (*ctc_arith_constant_provider_fn)(
    const void *context,
    uint32_t subround_index,
    uint32_t lane_index,
    ctc_arith_lane_constants_t *constants_out
);

typedef struct ctc_standard_constant_context {
    const char *label;
    uint32_t round_index;
} ctc_standard_constant_context_t;

typedef struct ctc_encoder_constant_context {
    uint32_t round_index;
    uint32_t block_index;
} ctc_encoder_constant_context_t;

static ctc_status_t ctc_make_label(
    const char *base_label,
    const char *suffix,
    char output[96]
) {
    int written;

    if (base_label == NULL || suffix == NULL || output == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    written = snprintf(output, 96U, "%s|%s", base_label, suffix);
    if (written < 0 || written >= 96) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    return CTC_STATUS_OK;
}

static uint32_t ctc_primary_index(uint32_t round_index, uint32_t subround_index) {
    return ((round_index & UINT32_C(0xFFFF)) << 16U)
        | (subround_index & UINT32_C(0xFFFF));
}

static uint32_t ctc_secondary_index(uint32_t lane_index, uint32_t purpose_index) {
    return ((lane_index & UINT32_C(0x00FFFFFF)) << 8U)
        | (purpose_index & UINT32_C(0xFF));
}

static ctc_status_t ctc_sbox_constants(
    const char *label,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t *constant_a,
    uint64_t *constant_b,
    uint64_t *constant_c
) {
    char derived_label[96];
    ctc_status_t status;
    const uint32_t primary = ctc_primary_index(round_index, subround_index);

    status = ctc_make_label(label, "SBOX-A", derived_label);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_constant_derive(
        derived_label,
        primary,
        ctc_secondary_index(lane_index, 0U),
        constant_a
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    status = ctc_make_label(label, "SBOX-B", derived_label);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_constant_derive_nonzero(
        derived_label,
        primary,
        ctc_secondary_index(lane_index, 1U),
        constant_b
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    status = ctc_make_label(label, "SBOX-C", derived_label);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_constant_derive(
        derived_label,
        primary,
        ctc_secondary_index(lane_index, 2U),
        constant_c
    );
}

static ctc_status_t ctc_round_constant(
    const char *label,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t *constant_out
) {
    char derived_label[96];
    ctc_status_t status = ctc_make_label(label, "ROUND-CONSTANT", derived_label);

    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_constant_derive(
        derived_label,
        ctc_primary_index(round_index, subround_index),
        ctc_secondary_index(lane_index, 0U),
        constant_out
    );
}

static ctc_status_t ctc_standard_constant_provider(
    const void *context,
    uint32_t subround_index,
    uint32_t lane_index,
    ctc_arith_lane_constants_t *constants_out
) {
    const ctc_standard_constant_context_t *standard_context =
        (const ctc_standard_constant_context_t *)context;
    ctc_status_t status;

    if (standard_context == NULL || standard_context->label == NULL
        || constants_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_round_constant(
        standard_context->label,
        standard_context->round_index,
        subround_index,
        lane_index,
        &constants_out->round_constant
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_sbox_constants(
        standard_context->label,
        standard_context->round_index,
        subround_index,
        lane_index,
        &constants_out->sbox_a,
        &constants_out->sbox_b,
        &constants_out->sbox_c
    );
}

static ctc_status_t ctc_encoder_constant_provider(
    const void *context,
    uint32_t subround_index,
    uint32_t lane_index,
    ctc_arith_lane_constants_t *constants_out
) {
    const ctc_encoder_constant_context_t *encoder_context =
        (const ctc_encoder_constant_context_t *)context;
    ctc_status_t status;

    if (encoder_context == NULL || constants_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_RC,
        encoder_context->round_index,
        encoder_context->block_index,
        subround_index,
        lane_index,
        &constants_out->round_constant
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_SBOX_A,
        encoder_context->round_index,
        encoder_context->block_index,
        subround_index,
        lane_index,
        &constants_out->sbox_a
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_SBOX_B,
        encoder_context->round_index,
        encoder_context->block_index,
        subround_index,
        lane_index,
        &constants_out->sbox_b
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_SBOX_C,
        encoder_context->round_index,
        encoder_context->block_index,
        subround_index,
        lane_index,
        &constants_out->sbox_c
    );
}

static ctc_uint128_t ctc_mod_inverse_u128(ctc_uint128_t value, ctc_uint128_t modulus) {
    ctc_int128_t old_remainder = (ctc_int128_t)modulus;
    ctc_int128_t remainder = (ctc_int128_t)(value % modulus);
    ctc_int128_t old_coefficient = 0;
    ctc_int128_t coefficient = 1;

    while (remainder != 0) {
        const ctc_int128_t quotient = old_remainder / remainder;
        const ctc_int128_t next_remainder = old_remainder - quotient * remainder;
        const ctc_int128_t next_coefficient = old_coefficient - quotient * coefficient;
        old_remainder = remainder;
        remainder = next_remainder;
        old_coefficient = coefficient;
        coefficient = next_coefficient;
    }

    if (old_remainder != 1) {
        return 0U;
    }
    if (old_coefficient < 0) {
        old_coefficient += (ctc_int128_t)modulus;
    }
    return (ctc_uint128_t)old_coefficient;
}

static void ctc_dickson_pair_u128(
    uint64_t value,
    ctc_uint128_t degree,
    uint64_t *current_out,
    uint64_t *next_out
) {
    if (degree == 0U) {
        *current_out = 2U;
        *next_out = ctc_field_reduce_u64(value);
        return;
    }

    uint64_t half_value;
    uint64_t half_next;
    uint64_t even_value;
    uint64_t odd_value;
    const ctc_uint128_t half_degree = degree >> 1U;

    ctc_dickson_pair_u128(value, half_degree, &half_value, &half_next);
    even_value = ctc_field_sub(ctc_field_mul(half_value, half_value), 2U);
    odd_value = ctc_field_sub(ctc_field_mul(half_value, half_next), value);

    if ((degree & 1U) == 0U) {
        *current_out = even_value;
        *next_out = odd_value;
    } else {
        *current_out = odd_value;
        *next_out = ctc_field_sub(ctc_field_mul(half_next, half_next), 2U);
    }
}

static uint64_t ctc_dickson_u128(uint64_t value, ctc_uint128_t degree) {
    uint64_t current;
    uint64_t next;

    ctc_dickson_pair_u128(value, degree, &current, &next);
    return current;
}

static ctc_uint128_t ctc_dickson_inverse_degree(uint64_t degree) {
    const ctc_uint128_t modulus =
        (ctc_uint128_t)CTC_FIELD_MODULUS * (ctc_uint128_t)CTC_FIELD_MODULUS - 1U;
    return ctc_mod_inverse_u128((ctc_uint128_t)degree, modulus);
}

static uint64_t ctc_sbox_apply_with_constants(
    uint64_t value,
    uint64_t degree,
    const ctc_arith_lane_constants_t *constants
) {
    uint64_t transformed = ctc_field_add(value, constants->sbox_c);

    transformed = ctc_field_dickson(transformed, degree);
    transformed = ctc_field_inv(transformed);
    transformed = ctc_field_mul(constants->sbox_b, transformed);
    return ctc_field_add(constants->sbox_a, transformed);
}

static ctc_status_t ctc_sbox_inverse_with_constants(
    uint64_t value,
    uint64_t degree,
    const ctc_arith_lane_constants_t *constants,
    uint64_t *result_out
) {
    const ctc_uint128_t inverse_degree = ctc_dickson_inverse_degree(degree);
    uint64_t transformed;

    if (constants == NULL || result_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (inverse_degree == 0U || constants->sbox_b == 0U) {
        return CTC_STATUS_INTERNAL_ERROR;
    }

    transformed = ctc_field_sub(value, constants->sbox_a);
    transformed = ctc_field_mul(transformed, ctc_field_inv(constants->sbox_b));
    transformed = ctc_field_inv(transformed);
    transformed = ctc_dickson_u128(transformed, inverse_degree);
    *result_out = ctc_field_sub(transformed, constants->sbox_c);
    return CTC_STATUS_OK;
}

ctc_status_t ctc_sbox_apply(
    const char *label,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t value,
    uint64_t *result_out
) {
    ctc_arith_lane_constants_t constants = {0U, 0U, 0U, 0U};
    const uint64_t degree = (subround_index & 1U) == 0U ? 23U : 47U;
    ctc_status_t status;

    if (label == NULL || result_out == NULL || lane_index >= CTC_BRANCH_LANES) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_sbox_constants(
        label,
        round_index,
        subround_index,
        lane_index,
        &constants.sbox_a,
        &constants.sbox_b,
        &constants.sbox_c
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    *result_out = ctc_sbox_apply_with_constants(value, degree, &constants);
    return CTC_STATUS_OK;
}

ctc_status_t ctc_sbox_inverse(
    const char *label,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t value,
    uint64_t *result_out
) {
    ctc_arith_lane_constants_t constants = {0U, 0U, 0U, 0U};
    const uint64_t degree = (subround_index & 1U) == 0U ? 23U : 47U;
    ctc_status_t status;

    if (label == NULL || result_out == NULL || lane_index >= CTC_BRANCH_LANES) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_sbox_constants(
        label,
        round_index,
        subround_index,
        lane_index,
        &constants.sbox_a,
        &constants.sbox_b,
        &constants.sbox_c
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_sbox_inverse_with_constants(value, degree, &constants, result_out);
}

ctc_status_t ctc_mds_matrix(uint64_t matrix_out[8][8]) {
    if (matrix_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    for (uint32_t row = 0U; row < CTC_BRANCH_LANES; ++row) {
        for (uint32_t column = 0U; column < CTC_BRANCH_LANES; ++column) {
            const uint64_t denominator = (uint64_t)(row + 1U) + (uint64_t)(column + 17U);
            matrix_out[row][column] = ctc_field_inv(denominator);
        }
    }
    return CTC_STATUS_OK;
}

ctc_status_t ctc_mds_inverse(uint64_t matrix_out[8][8]) {
    uint64_t augmented[8][16];
    ctc_status_t status;

    if (matrix_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_mds_matrix(matrix_out);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t row = 0U; row < CTC_BRANCH_LANES; ++row) {
        for (uint32_t column = 0U; column < CTC_BRANCH_LANES; ++column) {
            augmented[row][column] = matrix_out[row][column];
            augmented[row][column + CTC_BRANCH_LANES] = row == column ? 1U : 0U;
        }
    }

    for (uint32_t pivot_column = 0U; pivot_column < CTC_BRANCH_LANES; ++pivot_column) {
        uint32_t pivot_row = pivot_column;
        while (pivot_row < CTC_BRANCH_LANES && augmented[pivot_row][pivot_column] == 0U) {
            ++pivot_row;
        }
        if (pivot_row == CTC_BRANCH_LANES) {
            return CTC_STATUS_INTERNAL_ERROR;
        }
        if (pivot_row != pivot_column) {
            for (uint32_t column = 0U; column < 2U * CTC_BRANCH_LANES; ++column) {
                const uint64_t temporary = augmented[pivot_column][column];
                augmented[pivot_column][column] = augmented[pivot_row][column];
                augmented[pivot_row][column] = temporary;
            }
        }

        const uint64_t pivot_inverse = ctc_field_inv(augmented[pivot_column][pivot_column]);
        for (uint32_t column = 0U; column < 2U * CTC_BRANCH_LANES; ++column) {
            augmented[pivot_column][column] =
                ctc_field_mul(augmented[pivot_column][column], pivot_inverse);
        }

        for (uint32_t row = 0U; row < CTC_BRANCH_LANES; ++row) {
            if (row == pivot_column) {
                continue;
            }
            const uint64_t factor = augmented[row][pivot_column];
            if (factor == 0U) {
                continue;
            }
            for (uint32_t column = 0U; column < 2U * CTC_BRANCH_LANES; ++column) {
                augmented[row][column] = ctc_field_sub(
                    augmented[row][column],
                    ctc_field_mul(factor, augmented[pivot_column][column])
                );
            }
        }
    }

    for (uint32_t row = 0U; row < CTC_BRANCH_LANES; ++row) {
        for (uint32_t column = 0U; column < CTC_BRANCH_LANES; ++column) {
            matrix_out[row][column] = augmented[row][column + CTC_BRANCH_LANES];
        }
    }
    return CTC_STATUS_OK;
}

static void ctc_matrix_vector_mul(
    uint64_t matrix[8][8],
    const uint64_t input[8],
    uint64_t output[8]
) {
    for (uint32_t row = 0U; row < CTC_BRANCH_LANES; ++row) {
        uint64_t accumulator = 0U;
        for (uint32_t column = 0U; column < CTC_BRANCH_LANES; ++column) {
            accumulator = ctc_field_add(
                accumulator,
                ctc_field_mul(matrix[row][column], input[column])
            );
        }
        output[row] = accumulator;
    }
}

static ctc_status_t ctc_arith_apply_with_provider(
    uint64_t state[8],
    uint32_t subround_count,
    ctc_arith_constant_provider_fn provider,
    const void *provider_context
) {
    uint64_t matrix[8][8];
    ctc_status_t status;

    if (state == NULL || provider == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_mds_matrix(matrix);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t subround = 0U; subround < subround_count; ++subround) {
        const uint64_t degree = (subround & 1U) == 0U ? 23U : 47U;
        uint64_t nonlinear[8];
        uint64_t mixed[8];

        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            ctc_arith_lane_constants_t constants;

            status = provider(provider_context, subround, lane, &constants);
            if (status != CTC_STATUS_OK) {
                return status;
            }
            nonlinear[lane] = ctc_sbox_apply_with_constants(
                ctc_field_add(state[lane], constants.round_constant),
                degree,
                &constants
            );
        }
        ctc_matrix_vector_mul(matrix, nonlinear, mixed);
        memcpy(state, mixed, sizeof(mixed));
    }
    return CTC_STATUS_OK;
}

static ctc_status_t ctc_arith_inverse_with_provider(
    uint64_t state[8],
    uint32_t subround_count,
    ctc_arith_constant_provider_fn provider,
    const void *provider_context
) {
    uint64_t inverse_matrix[8][8];
    ctc_status_t status;

    if (state == NULL || provider == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_mds_inverse(inverse_matrix);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t remaining = subround_count; remaining > 0U; --remaining) {
        const uint32_t subround = remaining - 1U;
        const uint64_t degree = (subround & 1U) == 0U ? 23U : 47U;
        uint64_t unmixed[8];
        uint64_t restored[8];

        ctc_matrix_vector_mul(inverse_matrix, state, unmixed);
        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            ctc_arith_lane_constants_t constants;

            status = provider(provider_context, subround, lane, &constants);
            if (status != CTC_STATUS_OK) {
                return status;
            }
            status = ctc_sbox_inverse_with_constants(
                unmixed[lane],
                degree,
                &constants,
                &restored[lane]
            );
            if (status != CTC_STATUS_OK) {
                return status;
            }
            restored[lane] = ctc_field_sub(restored[lane], constants.round_constant);
        }
        memcpy(state, restored, sizeof(restored));
    }
    return CTC_STATUS_OK;
}

ctc_status_t ctc_arith_apply(
    const char *label,
    uint32_t round_index,
    uint64_t state[8],
    uint32_t subround_count
) {
    const ctc_standard_constant_context_t context = {label, round_index};

    if (label == NULL || state == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    return ctc_arith_apply_with_provider(
        state,
        subround_count,
        ctc_standard_constant_provider,
        &context
    );
}

ctc_status_t ctc_arith_inverse(
    const char *label,
    uint32_t round_index,
    uint64_t state[8],
    uint32_t subround_count
) {
    const ctc_standard_constant_context_t context = {label, round_index};

    if (label == NULL || state == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    return ctc_arith_inverse_with_provider(
        state,
        subround_count,
        ctc_standard_constant_provider,
        &context
    );
}

ctc_status_t ctc_arith_apply_encoder(
    uint32_t round_index,
    uint32_t block_index,
    uint64_t state[8],
    uint32_t subround_count
) {
    const ctc_encoder_constant_context_t context = {round_index, block_index};

    if (state == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS
        || block_index >= CTC_ENCODER_MAX_BLOCKS
        || subround_count > CTC_ARITH_ENCODER_SUBROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    return ctc_arith_apply_with_provider(
        state,
        subround_count,
        ctc_encoder_constant_provider,
        &context
    );
}

ctc_status_t ctc_arith_inverse_encoder(
    uint32_t round_index,
    uint32_t block_index,
    uint64_t state[8],
    uint32_t subround_count
) {
    const ctc_encoder_constant_context_t context = {round_index, block_index};

    if (state == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS
        || block_index >= CTC_ENCODER_MAX_BLOCKS
        || subround_count > CTC_ARITH_ENCODER_SUBROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    return ctc_arith_inverse_with_provider(
        state,
        subround_count,
        ctc_encoder_constant_provider,
        &context
    );
}
