#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "ctc_sigma/ctc_sigma.h"

typedef struct encoder_component_preview {
    const char *name;
    ctc_encoder_constant_component_t component;
} encoder_component_preview_t;

int main(void) {
    static const char *labels[] = {
        "IV-HASH",
        "IV-XOF",
        "FOLD-IV",
        "BRANCH-C"
    };
    static const encoder_component_preview_t encoder_components[] = {
        {"RC", CTC_ENCODER_CONSTANT_RC},
        {"SBOX-A", CTC_ENCODER_CONSTANT_SBOX_A},
        {"SBOX-B", CTC_ENCODER_CONSTANT_SBOX_B},
        {"SBOX-C", CTC_ENCODER_CONSTANT_SBOX_C}
    };

    printf("CTC-Sigma %s public constant preview\n", ctc_sigma_version());
    printf("\nPreserved v0.1 domains:\n");
    for (size_t label_index = 0U;
         label_index < sizeof(labels) / sizeof(labels[0]);
         ++label_index) {
        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            uint64_t value = 0U;
            const ctc_status_t status = ctc_constant_derive(
                labels[label_index],
                0U,
                lane,
                &value
            );
            if (status != CTC_STATUS_OK) {
                fprintf(stderr, "constant derivation failed: %s\n", ctc_status_string(status));
                return 1;
            }
            printf("%-12s lane=%" PRIu32 " value=%" PRIu64 "\n",
                   labels[label_index], lane, value);
        }
    }

    printf("\nA_ENC tweak preview (round=0, block=0, subround=0):\n");
    for (size_t component_index = 0U;
         component_index < sizeof(encoder_components) / sizeof(encoder_components[0]);
         ++component_index) {
        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            uint64_t value = 0U;
            const ctc_status_t status = ctc_encoder_constant_derive(
                encoder_components[component_index].component,
                0U,
                0U,
                0U,
                lane,
                &value
            );
            if (status != CTC_STATUS_OK) {
                fprintf(stderr, "encoder constant derivation failed: %s\n",
                        ctc_status_string(status));
                return 1;
            }
            printf("%-12s lane=%" PRIu32 " value=%" PRIu64 "\n",
                   encoder_components[component_index].name, lane, value);
        }
    }
    return 0;
}
