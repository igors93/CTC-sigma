#ifndef CTC_SIGMA_CONSTANTS_H
#define CTC_SIGMA_CONSTANTS_H

#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ctc_encoder_constant_component {
    CTC_ENCODER_CONSTANT_RC = 0,
    CTC_ENCODER_CONSTANT_SBOX_A = 1,
    CTC_ENCODER_CONSTANT_SBOX_B = 2,
    CTC_ENCODER_CONSTANT_SBOX_C = 3
} ctc_encoder_constant_component_t;

typedef enum ctc_v03_constant_purpose {
    CTC_V03_CONSTANT_GENERIC = 0,
    CTC_V03_CONSTANT_ROUND = 1,
    CTC_V03_CONSTANT_SBOX_A = 2,
    CTC_V03_CONSTANT_SBOX_B = 3,
    CTC_V03_CONSTANT_SBOX_C = 4,
    CTC_V03_CONSTANT_DESCRIPTOR_IV = 5,
    CTC_V03_CONSTANT_DESCRIPTOR_GROUP = 6,
    CTC_V03_CONSTANT_DYNAMIC_RC = 7,
    CTC_V03_CONSTANT_DYNAMIC_A = 8,
    CTC_V03_CONSTANT_DYNAMIC_B = 9,
    CTC_V03_CONSTANT_DYNAMIC_C = 10,
    CTC_V03_CONSTANT_SPONGE_IV = 11
} ctc_v03_constant_purpose_t;

/*
 * Compatibility entry point backed by the lossless v0.3 seed encoding.
 * first_index and second_index are serialized independently as LE32 values.
 */
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
 * Lossless CTC-Sigma v0.3 public constant domain.
 *
 * Seed = "CTC-SIGMA-v0.3|CONST|" || LEN8(component) || component
 *        || LE32(purpose) || LE32(round) || LE32(subround)
 *        || LE32(lane) || LE32(block)
 */
ctc_status_t ctc_v03_constant_derive(
    const char *component,
    ctc_v03_constant_purpose_t purpose,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t lane_index,
    uint32_t block_index,
    uint64_t *constant_out
);

/* Derive all eight lane constants with one SHAKE256 invocation. */
ctc_status_t ctc_v03_constant_derive_lanes(
    const char *component,
    ctc_v03_constant_purpose_t purpose,
    uint32_t round_index,
    uint32_t subround_index,
    uint32_t block_index,
    uint64_t constants_out[8]
);

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
