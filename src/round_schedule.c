#include "ctc_sigma/round_schedule.h"

#include <stdint.h>

#include "ctc_sigma/parameters.h"

ctc_status_t ctc_round_schedule_get(
    uint32_t round_index,
    ctc_round_type_t *round_type_out
) {
    static const ctc_round_type_t schedule[CTC_FEISTEL_ROUNDS] = {
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_ARITHMETIC_BRIDGE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_ARITHMETIC_BRIDGE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_ARITHMETIC_BRIDGE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_ARITHMETIC_BRIDGE
    };

    if (round_type_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    *round_type_out = schedule[round_index];
    return CTC_STATUS_OK;
}
