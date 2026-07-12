#ifndef CTC_SIGMA_ROUND_SCHEDULE_H
#define CTC_SIGMA_ROUND_SCHEDULE_H

#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ctc_round_type {
    CTC_ROUND_BRAID_ACTIVE = 0,
    CTC_ROUND_ARITHMETIC_BRIDGE = 1
} ctc_round_type_t;

ctc_status_t ctc_round_schedule_get(
    uint32_t round_index,
    ctc_round_type_t *round_type_out
);

#ifdef __cplusplus
}
#endif

#endif
