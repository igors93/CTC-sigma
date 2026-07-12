#include "ctc_sigma/constants.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/parameters.h"
#include "internal/keccak.h"
#include "internal/util.h"

#if !defined(__SIZEOF_INT128__)
#error "CTC-Sigma requires unsigned __int128 support."
#endif

__extension__ typedef unsigned __int128 ctc_uint128_t;

#define CTC_V03_COMPONENT_MAX_LENGTH 63U
#define CTC_V03_SEED_PREFIX "CTC-SIGMA-v0.3|CONST|"
#define CTC_V03_SEED_FIXED_BYTES (1U + 5U * 4U)

typedef struct ctc_encoder_component_definition {
    const char *name;
    ctc_v03_constant_purpose_t purpose;
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
            definition_out->name = "A_ENC";
            definition_out->purpose = CTC_V03_CONSTANT_ROUND;
            definition_out->must_be_nonzero = 0;
            return CTC_STATUS_OK;
        case CTC_ENCODER_CONSTANT_SBOX_A:
            definition_out->name = "A_ENC";
            definition_out->purpose = CTC_V03_CONSTANT_SBOX_A;
            definition_out->must_be_nonzero = 0;
            return CTC_STATUS_OK;
        case CTC_ENCODER_CONSTANT_SBOX_B:
            definition_out->name = "A_ENC";
            definition_out->purpose = CTC_V03_CONSTANT_SBOX_B;
            definition_out->must_be_nonzero = 1;
            return CTC_STATUS_OK;
        case CTC_ENCODER_CONSTANT_SBOX_C:
            definition_out->name = "A_ENC";
            definition_out->purpose = CTC_V03_CONSTANT_SBOX_C;
            definition_out->must_be_nonzero = 0;
            return CTC_STATUS_OK;
        default:
            return CTC_STATUS_INVALID_ARGUMENT;
    }
}

static ctc_status_t ctc_v03_build_seed(
    const char *component,
    ctc_v03_constant_purpose_t purpose,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint32_t block_index,
    uint8_t seed_out[
        (sizeof(CTC_V03_SEED_PREFIX) - 1U) + CTC_V03_COMPONENT_MAX_LENGTH
        + CTC_V03_SEED_FIXED_BYTES
    ],
    size_t *seed_length_out
) {
    const size_t prefix_length = sizeof(CTC_V03_SEED_PREFIX) - 1U;
    const size_t component_length = component == NULL ? 0U : strlen(component);
    size_t offset = 0U;

    if (component == NULL || seed_out == NULL || seed_length_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (component_length == 0U || component_length > CTC_V03_COMPONENT_MAX_LENGTH) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    if ((uint32_t)purpose > (uint32_t)CTC_V03_CONSTANT_SPONGE_IV) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    memcpy(seed_out + offset, CTC_V03_SEED_PREFIX, prefix_length);
    offset += prefix_length;
    seed_out[offset++] = (uint8_t)component_length;
    memcpy(seed_out + offset, component, component_length);
    offset += component_length;
    ctc_store_le32(seed_out + offset, (uint32_t)purpose);
    offset += 4U;
    ctc_store_le32(seed_out + offset, round_index);
    offset += 4U;
    ctc_store_le32(seed_out + offset, subround_index);
    offset += 4U;
    ctc_store_le32(seed_out + offset, lane_index);
    offset += 4U;
    ctc_store_le32(seed_out + offset, block_index);
    offset += 4U;

    *seed_length_out = offset;
    return CTC_STATUS_OK;
}

ctc_status_t ctc_v03_constant_derive(
    const char *component,
    ctc_v03_constant_purpose_t purpose,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint32_t block_index,
    uint64_t *constant_out
) {
    uint8_t seed[
        (sizeof(CTC_V03_SEED_PREFIX) - 1U) + CTC_V03_COMPONENT_MAX_LENGTH
        + CTC_V03_SEED_FIXED_BYTES
    ];
    uint8_t output[16];
    size_t seed_length = 0U;
    ctc_status_t status;

    if (constant_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_v03_build_seed(
        component,
        purpose,
        round_index,
        subround_index,
        lane_index,
        block_index,
        seed,
        &seed_length
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    ctc_shake256(seed, seed_length, output, sizeof(output));
    *constant_out = ctc_constant_reduce_output(output);
    return CTC_STATUS_OK;
}

ctc_status_t ctc_v03_constant_derive_lanes(
    const char *component,
    ctc_v03_constant_purpose_t purpose,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t block_index,
    uint64_t constants_out[8]
) {
    uint8_t seed[
        (sizeof(CTC_V03_SEED_PREFIX) - 1U) + CTC_V03_COMPONENT_MAX_LENGTH
        + CTC_V03_SEED_FIXED_BYTES
    ];
    uint8_t output[8U * 16U];
    size_t seed_length = 0U;
    ctc_status_t status;

    if (constants_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_v03_build_seed(
        component,
        purpose,
        round_index,
        subround_index,
        UINT32_MAX,
        block_index,
        seed,
        &seed_length
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }

    ctc_shake256(seed, seed_length, output, sizeof(output));
    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        constants_out[lane] = ctc_constant_reduce_output(output + lane * 16U);
    }
    return CTC_STATUS_OK;
}

ctc_status_t ctc_constant_derive(
    const char *label,
    uint32_t first_index,
    uint32_t second_index,
    uint64_t *constant_out
) {
    return ctc_v03_constant_derive(
        label,
        CTC_V03_CONSTANT_GENERIC,
        first_index,
        second_index,
        0U,
        0U,
        constant_out
    );
}

ctc_status_t ctc_constant_derive_nonzero(
    const char *label,
    uint32_t first_index,
    uint32_t second_index,
    uint64_t *constant_out
) {
    ctc_status_t status = ctc_constant_derive(
        label,
        first_index,
        second_index,
        constant_out
    );

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
    ctc_encoder_component_definition_t definition;
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
    status = ctc_v03_constant_derive(
        definition.name,
        definition.purpose,
        round_index,
        subround_index,
        lane_index,
        block_index,
        constant_out
    );
    if (status == CTC_STATUS_OK && definition.must_be_nonzero != 0
        && *constant_out == 0U) {
        *constant_out = 1U;
    }
    return status;
}
