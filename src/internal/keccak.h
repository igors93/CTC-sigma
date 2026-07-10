#ifndef CTC_SIGMA_INTERNAL_KECCAK_H
#define CTC_SIGMA_INTERNAL_KECCAK_H

#include <stddef.h>
#include <stdint.h>

void ctc_shake256(
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_length
);

#endif
