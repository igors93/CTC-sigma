#include "ctc_sigma/field.h"

#include <stdint.h>

#include "ctc_sigma/parameters.h"

#if !defined(__SIZEOF_INT128__)
#error "This initial CTC-Sigma implementation requires unsigned __int128 support."
#endif

__extension__ typedef unsigned __int128 ctc_uint128_t;

static uint64_t ctc_field_reduce_u128(ctc_uint128_t value) {
    const ctc_uint128_t modulus = (ctc_uint128_t)CTC_FIELD_MODULUS;
    return (uint64_t)(value % modulus);
}

uint64_t ctc_field_reduce_u64(uint64_t value) {
    return value % CTC_FIELD_MODULUS;
}

uint64_t ctc_field_add(uint64_t left, uint64_t right) {
    const uint64_t normalized_left = ctc_field_reduce_u64(left);
    const uint64_t normalized_right = ctc_field_reduce_u64(right);
    const uint64_t sum = normalized_left + normalized_right;
    return sum >= CTC_FIELD_MODULUS ? sum - CTC_FIELD_MODULUS : sum;
}

uint64_t ctc_field_sub(uint64_t left, uint64_t right) {
    const uint64_t normalized_left = ctc_field_reduce_u64(left);
    const uint64_t normalized_right = ctc_field_reduce_u64(right);
    return normalized_left >= normalized_right
        ? normalized_left - normalized_right
        : CTC_FIELD_MODULUS - (normalized_right - normalized_left);
}

uint64_t ctc_field_mul(uint64_t left, uint64_t right) {
    return ctc_field_reduce_u128(
        (ctc_uint128_t)ctc_field_reduce_u64(left)
        * (ctc_uint128_t)ctc_field_reduce_u64(right)
    );
}

uint64_t ctc_field_pow(uint64_t base, uint64_t exponent) {
    uint64_t result = 1U;
    uint64_t current = ctc_field_reduce_u64(base);
    uint64_t remaining = exponent;

    while (remaining > 0U) {
        if ((remaining & UINT64_C(1)) != 0U) {
            result = ctc_field_mul(result, current);
        }
        current = ctc_field_mul(current, current);
        remaining >>= 1U;
    }
    return result;
}

uint64_t ctc_field_inv(uint64_t value) {
    const uint64_t normalized = ctc_field_reduce_u64(value);
    return normalized == 0U
        ? 0U
        : ctc_field_pow(normalized, CTC_FIELD_MODULUS - UINT64_C(2));
}

uint64_t ctc_field_dickson(uint64_t value, uint64_t degree) {
    const uint64_t x = ctc_field_reduce_u64(value);
    if (degree == 0U) {
        return 2U;
    }
    if (degree == 1U) {
        return x;
    }

    uint64_t previous = 2U;
    uint64_t current = x;
    for (uint64_t index = 2U; index <= degree; ++index) {
        const uint64_t next = ctc_field_sub(ctc_field_mul(x, current), previous);
        previous = current;
        current = next;
    }
    return current;
}
