#include "ctc_sigma/validation.h"

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/parameters.h"

ctc_status_t ctc_validate_canonical_lanes(
    const uint64_t *values,
    size_t count
) {
    if (values == NULL && count != 0U) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    for (size_t index = 0U; index < count; ++index) {
        if (values[index] >= CTC_FIELD_MODULUS) {
            return CTC_STATUS_OUT_OF_RANGE;
        }
    }
    return CTC_STATUS_OK;
}
