#ifndef CTC_SIGMA_SPONGE_H
#define CTC_SIGMA_SPONGE_H

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/braid.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ctc_domain {
    CTC_DOMAIN_HASH256 = 0x01,
    CTC_DOMAIN_XOF = 0x02,
    CTC_DOMAIN_INTERNAL_TEST = 0x7F
} ctc_domain_t;

typedef struct ctc_sponge {
    uint64_t state[16];
    ctc_domain_t domain;
} ctc_sponge_t;

ctc_status_t ctc_sponge_initialize(ctc_sponge_t *sponge, ctc_domain_t domain);

ctc_status_t ctc_sponge_encode_message(
    const uint8_t *message,
    size_t message_length,
    ctc_domain_t domain,
    uint8_t **encoded_message_out,
    size_t *encoded_length_out
);

void ctc_sponge_free_encoded_message(uint8_t *encoded_message);

ctc_status_t ctc_hash256(
    const uint8_t *message,
    size_t message_length,
    uint8_t digest_out[32]
);

ctc_status_t ctc_xof(
    const uint8_t *message,
    size_t message_length,
    uint8_t *output,
    size_t output_length
);

ctc_status_t ctc_hash256_with_normalizer(
    const uint8_t *message,
    size_t message_length,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint8_t digest_out[32]
);

ctc_status_t ctc_xof_with_normalizer(
    const uint8_t *message,
    size_t message_length,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint8_t *output,
    size_t output_length
);

#ifdef __cplusplus
}
#endif

#endif
