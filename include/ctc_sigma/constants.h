#ifndef CTC_SIGMA_CONSTANTS_H
#define CTC_SIGMA_CONSTANTS_H

#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
