#ifndef CTC_SIGMA_INTERNAL_UTIL_H
#define CTC_SIGMA_INTERNAL_UTIL_H

#include <stdint.h>

static inline void ctc_store_le32(uint8_t output[4], uint32_t value) {
    output[0] = (uint8_t)(value & UINT32_C(0xFF));
    output[1] = (uint8_t)((value >> 8U) & UINT32_C(0xFF));
    output[2] = (uint8_t)((value >> 16U) & UINT32_C(0xFF));
    output[3] = (uint8_t)((value >> 24U) & UINT32_C(0xFF));
}

static inline void ctc_store_le64(uint8_t output[8], uint64_t value) {
    for (uint32_t index = 0U; index < 8U; ++index) {
        output[index] = (uint8_t)((value >> (8U * index)) & UINT64_C(0xFF));
    }
}

static inline uint64_t ctc_load_le40(const uint8_t input[5]) {
    uint64_t value = 0U;
    for (uint32_t index = 0U; index < 5U; ++index) {
        value |= ((uint64_t)input[index]) << (8U * index);
    }
    return value;
}

static inline void ctc_store_le40(uint8_t output[5], uint64_t value) {
    for (uint32_t index = 0U; index < 5U; ++index) {
        output[index] = (uint8_t)((value >> (8U * index)) & UINT64_C(0xFF));
    }
}

#endif
