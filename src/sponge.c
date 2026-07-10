#include "ctc_sigma/sponge.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ctc_sigma/constants.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/permutation.h"
#include "internal/util.h"

static const char *ctc_domain_iv_label(ctc_domain_t domain) {
    switch (domain) {
        case CTC_DOMAIN_HASH256:
            return "IV-HASH";
        case CTC_DOMAIN_XOF:
            return "IV-XOF";
        case CTC_DOMAIN_INTERNAL_TEST:
            return "IV-TEST";
        default:
            return NULL;
    }
}

ctc_status_t ctc_sponge_initialize(ctc_sponge_t *sponge, ctc_domain_t domain) {
    const char *label;

    if (sponge == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    label = ctc_domain_iv_label(domain);
    if (label == NULL) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    memset(sponge, 0, sizeof(*sponge));
    sponge->domain = domain;
    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        ctc_status_t status = ctc_constant_derive(
            label,
            0U,
            lane,
            &sponge->state[CTC_RATE_LANES + lane]
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
    }
    return CTC_STATUS_OK;
}

ctc_status_t ctc_sponge_encode_message(
    const uint8_t *message,
    size_t message_length,
    ctc_domain_t domain,
    uint8_t **encoded_message_out,
    size_t *encoded_length_out
) {
    size_t base_length;
    size_t zero_count;
    size_t total_length;
    uint8_t *encoded;

    if ((message == NULL && message_length != 0U)
        || encoded_message_out == NULL
        || encoded_length_out == NULL
        || ctc_domain_iv_label(domain) == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (message_length > UINT64_MAX || message_length > SIZE_MAX - 11U) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    base_length = message_length + 8U + 1U + 1U;
    zero_count = (39U + CTC_RATE_BYTES - (base_length % CTC_RATE_BYTES)) % CTC_RATE_BYTES;
    if (base_length > SIZE_MAX - zero_count - 1U) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    total_length = base_length + zero_count + 1U;

    encoded = (uint8_t *)calloc(total_length, 1U);
    if (encoded == NULL) {
        return CTC_STATUS_ALLOCATION_FAILED;
    }

    if (message_length > 0U) {
        memcpy(encoded, message, message_length);
    }
    ctc_store_le64(encoded + message_length, (uint64_t)message_length);
    encoded[message_length + 8U] = (uint8_t)domain;
    encoded[message_length + 9U] = UINT8_C(0x01);
    encoded[total_length - 1U] = UINT8_C(0x80);

    *encoded_message_out = encoded;
    *encoded_length_out = total_length;
    return CTC_STATUS_OK;
}

void ctc_sponge_free_encoded_message(uint8_t *encoded_message) {
    free(encoded_message);
}

ctc_status_t ctc_sponge_rate_block_is_acceptable(
    const uint64_t rate_lanes[CTC_RATE_LANES],
    int *accepted_out
) {
    const uint64_t two_to_40 = UINT64_C(1) << 40U;
    const uint64_t threshold_40 =
        (CTC_FIELD_MODULUS / two_to_40) * two_to_40;

    if (rate_lanes == NULL || accepted_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    *accepted_out = 1;
    for (uint32_t lane = 0U; lane < CTC_RATE_LANES; ++lane) {
        if (rate_lanes[lane] >= CTC_FIELD_MODULUS) {
            return CTC_STATUS_OUT_OF_RANGE;
        }
        if (rate_lanes[lane] >= threshold_40) {
            *accepted_out = 0;
        }
    }
    return CTC_STATUS_OK;
}

static ctc_status_t ctc_sponge_absorb(
    ctc_sponge_t *sponge,
    const uint8_t *encoded_message,
    size_t encoded_length,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context
) {
    if (sponge == NULL
        || encoded_message == NULL
        || encoded_length == 0U
        || encoded_length % CTC_RATE_BYTES != 0U
        || normalizer == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    for (size_t offset = 0U; offset < encoded_length; offset += CTC_RATE_BYTES) {
        for (uint32_t lane = 0U; lane < CTC_RATE_LANES; ++lane) {
            const uint64_t word = ctc_load_le40(encoded_message + offset + lane * 5U);
            sponge->state[lane] = ctc_field_add(sponge->state[lane], word);
        }
        const ctc_status_t status = ctc_permutation_apply_with_normalizer(
            sponge->state,
            normalizer,
            normalizer_context
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
    }
    return CTC_STATUS_OK;
}

static ctc_status_t ctc_sponge_squeeze(
    ctc_sponge_t *sponge,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint8_t *output,
    size_t output_length
) {
    const uint64_t two_to_40 = UINT64_C(1) << 40U;
    size_t produced = 0U;

    if (sponge == NULL
        || normalizer == NULL
        || (output == NULL && output_length != 0U)) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    while (produced < output_length) {
        uint32_t rejection_count = 0U;
        int accepted = 0;

        while (accepted == 0 && rejection_count < CTC_SQUEEZE_MAX_REJECTIONS) {
            ctc_status_t status = ctc_sponge_rate_block_is_acceptable(
                sponge->state,
                &accepted
            );
            if (status != CTC_STATUS_OK) {
                return status;
            }
            if (accepted == 0) {
                status = ctc_permutation_apply_with_normalizer(
                    sponge->state,
                    normalizer,
                    normalizer_context
                );
                if (status != CTC_STATUS_OK) {
                    return status;
                }
                ++rejection_count;
            }
        }
        if (accepted == 0) {
            return CTC_STATUS_REJECTION_LIMIT;
        }

        for (uint32_t lane = 0U;
             lane < CTC_RATE_LANES && produced < output_length;
             ++lane) {
            uint8_t lane_bytes[5];
            const size_t remaining = output_length - produced;
            const size_t copied = remaining < 5U ? remaining : 5U;
            ctc_store_le40(lane_bytes, sponge->state[lane] & (two_to_40 - 1U));
            memcpy(output + produced, lane_bytes, copied);
            produced += copied;
        }

        if (produced < output_length) {
            const ctc_status_t status = ctc_permutation_apply_with_normalizer(
                sponge->state,
                normalizer,
                normalizer_context
            );
            if (status != CTC_STATUS_OK) {
                return status;
            }
        }
    }
    return CTC_STATUS_OK;
}

static ctc_status_t ctc_sponge_hash_internal(
    const uint8_t *message,
    size_t message_length,
    ctc_domain_t domain,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint8_t *output,
    size_t output_length
) {
    ctc_sponge_t sponge;
    uint8_t *encoded_message = NULL;
    size_t encoded_length = 0U;
    ctc_status_t status;

    if (normalizer == NULL || (output == NULL && output_length != 0U)) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    status = ctc_sponge_initialize(&sponge, domain);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_sponge_encode_message(
        message,
        message_length,
        domain,
        &encoded_message,
        &encoded_length
    );
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_sponge_absorb(
        &sponge,
        encoded_message,
        encoded_length,
        normalizer,
        normalizer_context
    );
    ctc_sponge_free_encoded_message(encoded_message);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    return ctc_sponge_squeeze(
        &sponge,
        normalizer,
        normalizer_context,
        output,
        output_length
    );
}

ctc_status_t ctc_hash256_with_normalizer(
    const uint8_t *message,
    size_t message_length,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint8_t digest_out[32]
) {
    return ctc_sponge_hash_internal(
        message,
        message_length,
        CTC_DOMAIN_HASH256,
        normalizer,
        normalizer_context,
        digest_out,
        CTC_HASH256_BYTES
    );
}

ctc_status_t ctc_xof_with_normalizer(
    const uint8_t *message,
    size_t message_length,
    ctc_braid_normalizer_fn normalizer,
    void *normalizer_context,
    uint8_t *output,
    size_t output_length
) {
    return ctc_sponge_hash_internal(
        message,
        message_length,
        CTC_DOMAIN_XOF,
        normalizer,
        normalizer_context,
        output,
        output_length
    );
}

ctc_status_t ctc_hash256(
    const uint8_t *message,
    size_t message_length,
    uint8_t digest_out[32]
) {
    return ctc_hash256_with_normalizer(
        message,
        message_length,
        ctc_braid_normalize_left,
        NULL,
        digest_out
    );
}

ctc_status_t ctc_xof(
    const uint8_t *message,
    size_t message_length,
    uint8_t *output,
    size_t output_length
) {
    return ctc_xof_with_normalizer(
        message,
        message_length,
        ctc_braid_normalize_left,
        NULL,
        output,
        output_length
    );
}
