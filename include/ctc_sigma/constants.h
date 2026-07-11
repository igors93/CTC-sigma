#ifndef CTC_SIGMA_CONSTANTS_H
#define CTC_SIGMA_CONSTANTS_H

#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Components of the CTC-Sigma v0.2 tweakable encoder constant domain.
 *
 * The numeric values are API identifiers only. The normative seed encodes the
 * canonical ASCII names "RC", "SBOX-A", "SBOX-B", and "SBOX-C".
 */
typedef enum ctc_encoder_constant_component {
    CTC_ENCODER_CONSTANT_RC = 0,
    CTC_ENCODER_CONSTANT_SBOX_A = 1,
    CTC_ENCODER_CONSTANT_SBOX_B = 2,
    CTC_ENCODER_CONSTANT_SBOX_C = 3
} ctc_encoder_constant_component_t;

ctc_status_t ctc_constant_derive(
    const char *label,
    uint32_t first_index,
    uint32_t second_index,
    uint64_t *constant_out
);

ctc_status_t ctc_constant_derive_nonzero(
    const char *label,
    uint32_t first_index,
    uint32_t second_index,
    uint64_t *constant_out
);

/*
 * Derive one public constant for A_ENC_{round,block}.
 *
 * SeedEnc(c,i,h,s,j) =
 *   ASCII("CTC-SIGMA-v0.2|A_ENC-TWEAK|") || LEN8(c) || ASCII(c)
 *   || LE32(i) || LE32(h) || LE32(s) || LE32(j)
 *
 * SBOX-B is a multiplicative constant and is therefore mapped from zero to
 * one. All indices are range-checked before serialization; no truncation is
 * permitted in this domain.
 */
ctc_status_t ctc_encoder_constant_derive(
    ctc_encoder_constant_component_t component,
    uint32_t round_index,
    uint32_t block_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint64_t *constant_out
);

#ifdef __cplusplus
}
#endif

#endif
