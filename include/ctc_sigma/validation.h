#ifndef CTC_SIGMA_VALIDATION_H
#define CTC_SIGMA_VALIDATION_H

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

ctc_status_t ctc_validate_canonical_lanes(
    const uint64_t *values,
    size_t count
);

#ifdef __cplusplus
}
#endif

#endif
