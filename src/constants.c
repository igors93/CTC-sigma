#include "ctc_sigma/constants.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ctc_sigma/parameters.h"
#include "internal/keccak.h"
#include "internal/util.h"

#if !defined(__SIZEOF_INT128__)
#error "This initial CTC-Sigma implementation requires unsigned __int128 support."
#endif

__extension__ typedef unsigned __int128 ctc_uint128_t;

#define CTC_ENCODER_COMPONENT_MAX_LENGTH 6U

typedef struct ctc_encoder_component_definition {
    const char *name;
    uint8_t length;
    int must_be_nonzero;
} ctc_encoder_component_definition_t;

static uint64_t ctc_constant_reduce_output(const uint8_t output[16]) {
    ctc_uint128_t integer_value = 0U;

    for (uint32_t index = 0U; index < 16U; ++index) {
        integer_value |= ((ctc_uint128_t)output[index]) << (8U * index);
    }
    return (uint64_t)(integer_value % (ctc_uint128_t)CTC_FIELD_MODULUS);
}

static ctc_status_t ctc_encoder_component_definition(
    ctc_encoder_constant_component_t component,
    ctc_encoder_component_definition_t *definition_out
) {
    if (definition_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    switch (component) {
        case CTC_ENCODER_CONSTANT_RC:
            definition_out->name = "RC";
            definition_out->length = UINT8_C(2);
            definition_out->must_be_nonzero = 0;
            return CTC_STATUS_OK;
        case CTC_ENCODER_CONSTANT_SBOX_A:
            definition_out->name = "SBOX-A";
            definition_out->length = UINT8_C(6);
            definition_out->must_be_nonzero = 0;
            return CTC_STATUS_OK;
        case CTC_ENCODER_CONSTANT_SBOX_B:
            definition_out->name = "SBOX-B";
            definition_out->length = UINT8_C(6);
            definition_out->must_be_nonzero = 1;
            return CTC_STATUS_OK;
        case CTC_ENCODER_CONSTANT_SBOX_C:
            definition_out->name = "SBOX-C";
            definition_out->length = UINT8_C(6);
            definition_out->must_be_nonzero = 0;
            return CTC_STATUS_OK;
        default:
            return CTC_STATUS_INVALID_ARGUMENT;
    }
}

ctc_status_t ctc_constant_derive(
    const char *label,
    uint32_t first_index,
    uint32_t second_index,
    uint64_t *constant_out
) {
    static const char prefix[] = "CTC-SIGMA-v0.1|";
    const size_t prefix_length = sizeof(prefix) - 1U;
    size_t label_length;
    size_t seed_length;
    uint8_t *seed;
    uint8_t output[16];

    if (label == NULL || constant_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    label_length = strlen(label);
    if (label_length > SIZE_MAX - prefix_length - 8U) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    seed_length = prefix_length + label_length + 8U;
    seed = (uint8_t *)malloc(seed_length);
    if (seed == NULL) {
        return CTC_STATUS_ALLOCATION_FAILED;
    }

    memcpy(seed, prefix, prefix_length);
    memcpy(seed + prefix_length, label, label_length);
    ctc_store_le32(seed + prefix_length + label_length, first_index);
    ctc_store_le32(seed + prefix_length + label_length + 4U, second_index);

    ctc_shake256(seed, seed_length, output, sizeof(output));
    free(seed);

    *constant_out = ctc_constant_reduce_output(output);
    return CTC_STATUS_OK;
}

ctc_status_t ctc_constant_derive_nonzero(
    const char *label,
    uint32_t first_index,
    uint32_t second_index,
    uint64_t *constant_out
) {
    ctc_status_t status = ctc_constant_derive(label, first_index, second_index, constant_out);
    if (status == CTC_STATUS_OK && *constant_out == 0U) {
        *constant_out = 1U;
    }
    return status;
}

ctc_status_t ctc_encoder_constant_derive(
    ctc_encoder_constant_component_t component,
    uint32_t round_index,
    uint32_t block_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t *constant_out
) {
    static const char prefix[] = "CTC-SIGMA-v0.2|A_ENC-TWEAK|";
    const size_t prefix_length = sizeof(prefix) - 1U;
    uint8_t seed[
        (sizeof(prefix) - 1U) + 1U + CTC_ENCODER_COMPONENT_MAX_LENGTH + 16U
    ];
    uint8_t output[16];
    ctc_encoder_component_definition_t definition;
    size_t offset = 0U;
    ctc_status_t status;

    if (constant_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS
        || block_index >= CTC_ENCODER_MAX_BLOCKS
        || subround_index >= CTC_ARITH_ENCODER_SUBROUNDS
        || lane_index >= CTC_BRANCH_LANES) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    status = ctc_encoder_component_definition(component, &definition);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memcpy(seed + offset, prefix, prefix_length);
    offset += prefix_length;
    seed[offset] = definition.length;
    ++offset;
    memcpy(seed + offset, definition.name, definition.length);
    offset += definition.length;
    ctc_store_le32(seed + offset, round_index);
    offset += 4U;
    ctc_store_le32(seed + offset, block_index);
    offset += 4U;
    ctc_store_le32(seed + offset, subround_index);
    offset += 4U;
    ctc_store_le32(seed + offset, lane_index);
    offset += 4U;

    ctc_shake256(seed, offset, output, sizeof(output));
    *constant_out = ctc_constant_reduce_output(output);
    if (definition.must_be_nonzero != 0 && *constant_out == 0U) {
        *constant_out = 1U;
    }
    return CTC_STATUS_OK;
}
