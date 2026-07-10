#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "ctc_sigma/ctc_sigma.h"

int main(void) {
    static const char *labels[] = {
        "IV-HASH",
        "IV-XOF",
        "FOLD-IV",
        "BRANCH-C"
    };

    printf("CTC-Sigma %s public constant preview\n", ctc_sigma_version());
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
    return 0;
}
