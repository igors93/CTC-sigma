#include "internal/keccak.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define CTC_KECCAK_ROUNDS 24U
#define CTC_SHAKE256_RATE 136U

static uint64_t ctc_rotate_left64(uint64_t value, unsigned int shift) {
    return (value << shift) | (value >> (64U - shift));
}

static uint64_t ctc_load_le64_local(const uint8_t input[8]) {
    uint64_t value = 0U;
    for (unsigned int index = 0U; index < 8U; ++index) {
        value |= ((uint64_t)input[index]) << (8U * index);
    }
    return value;
}

static void ctc_store_le64_local(uint8_t output[8], uint64_t value) {
    for (unsigned int index = 0U; index < 8U; ++index) {
        output[index] = (uint8_t)((value >> (8U * index)) & UINT64_C(0xFF));
    }
}

static void ctc_keccak_f1600(uint64_t state[25]) {
    static const uint64_t round_constants[CTC_KECCAK_ROUNDS] = {
        UINT64_C(0x0000000000000001), UINT64_C(0x0000000000008082),
        UINT64_C(0x800000000000808A), UINT64_C(0x8000000080008000),
        UINT64_C(0x000000000000808B), UINT64_C(0x0000000080000001),
        UINT64_C(0x8000000080008081), UINT64_C(0x8000000000008009),
        UINT64_C(0x000000000000008A), UINT64_C(0x0000000000000088),
        UINT64_C(0x0000000080008009), UINT64_C(0x000000008000000A),
        UINT64_C(0x000000008000808B), UINT64_C(0x800000000000008B),
        UINT64_C(0x8000000000008089), UINT64_C(0x8000000000008003),
        UINT64_C(0x8000000000008002), UINT64_C(0x8000000000000080),
        UINT64_C(0x000000000000800A), UINT64_C(0x800000008000000A),
        UINT64_C(0x8000000080008081), UINT64_C(0x8000000000008080),
        UINT64_C(0x0000000080000001), UINT64_C(0x8000000080008008)
    };
    static const unsigned int rotation_offsets[25] = {
         0U,  1U, 62U, 28U, 27U,
        36U, 44U,  6U, 55U, 20U,
         3U, 10U, 43U, 25U, 39U,
        41U, 45U, 15U, 21U,  8U,
        18U,  2U, 61U, 56U, 14U
    };

    for (unsigned int round = 0U; round < CTC_KECCAK_ROUNDS; ++round) {
        uint64_t column_parity[5];
        uint64_t theta_mix[5];
        uint64_t transformed[25];

        for (unsigned int x = 0U; x < 5U; ++x) {
            column_parity[x] = state[x] ^ state[x + 5U] ^ state[x + 10U]
                             ^ state[x + 15U] ^ state[x + 20U];
        }
        for (unsigned int x = 0U; x < 5U; ++x) {
            theta_mix[x] = column_parity[(x + 4U) % 5U]
                         ^ ctc_rotate_left64(column_parity[(x + 1U) % 5U], 1U);
        }
        for (unsigned int y = 0U; y < 5U; ++y) {
            for (unsigned int x = 0U; x < 5U; ++x) {
                state[x + 5U * y] ^= theta_mix[x];
            }
        }

        for (unsigned int y = 0U; y < 5U; ++y) {
            for (unsigned int x = 0U; x < 5U; ++x) {
                const unsigned int source = x + 5U * y;
                const unsigned int destination_x = y;
                const unsigned int destination_y = (2U * x + 3U * y) % 5U;
                const unsigned int offset = rotation_offsets[source];
                transformed[destination_x + 5U * destination_y] =
                    offset == 0U ? state[source] : ctc_rotate_left64(state[source], offset);
            }
        }

        for (unsigned int y = 0U; y < 5U; ++y) {
            for (unsigned int x = 0U; x < 5U; ++x) {
                state[x + 5U * y] = transformed[x + 5U * y]
                    ^ ((~transformed[((x + 1U) % 5U) + 5U * y])
                    & transformed[((x + 2U) % 5U) + 5U * y]);
            }
        }

        state[0] ^= round_constants[round];
    }
}

static void ctc_keccak_absorb_block(uint64_t state[25], const uint8_t block[CTC_SHAKE256_RATE]) {
    for (unsigned int lane = 0U; lane < CTC_SHAKE256_RATE / 8U; ++lane) {
        state[lane] ^= ctc_load_le64_local(block + lane * 8U);
    }
    ctc_keccak_f1600(state);
}

void ctc_shake256(
    const uint8_t *input,
    size_t input_length,
    uint8_t *output,
    size_t output_length
) {
    uint64_t state[25] = {0U};
    uint8_t block[CTC_SHAKE256_RATE];

    while (input_length >= CTC_SHAKE256_RATE) {
        ctc_keccak_absorb_block(state, input);
        input += CTC_SHAKE256_RATE;
        input_length -= CTC_SHAKE256_RATE;
    }

    memset(block, 0, sizeof(block));
    if (input_length > 0U) {
        memcpy(block, input, input_length);
    }
    block[input_length] ^= UINT8_C(0x1F);
    block[CTC_SHAKE256_RATE - 1U] ^= UINT8_C(0x80);
    ctc_keccak_absorb_block(state, block);

    while (output_length > 0U) {
        const size_t current_length = output_length < CTC_SHAKE256_RATE
            ? output_length
            : CTC_SHAKE256_RATE;

        for (unsigned int lane = 0U; lane < CTC_SHAKE256_RATE / 8U; ++lane) {
            ctc_store_le64_local(block + lane * 8U, state[lane]);
        }
        memcpy(output, block, current_length);
        output += current_length;
        output_length -= current_length;

        if (output_length > 0U) {
            ctc_keccak_f1600(state);
        }
    }
}
