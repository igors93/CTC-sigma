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
    ctc_uint128_t integer_value = 0U;

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

    for (uint32_t index = 0U; index < 16U; ++index) {
        integer_value |= ((ctc_uint128_t)output[index]) << (8U * index);
    }
    *constant_out = (uint64_t)(integer_value % (ctc_uint128_t)CTC_FIELD_MODULUS);
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
