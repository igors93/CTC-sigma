#include "ctc_sigma/ctc_sigma.h"

const char *ctc_sigma_version(void) {
    return CTC_SIGMA_VERSION_STRING;
}

const char *ctc_status_string(ctc_status_t status) {
    switch (status) {
        case CTC_STATUS_OK:
            return "ok";
        case CTC_STATUS_INVALID_ARGUMENT:
            return "invalid argument";
        case CTC_STATUS_OUT_OF_RANGE:
            return "out of range";
        case CTC_STATUS_BUFFER_TOO_SMALL:
            return "buffer too small";
        case CTC_STATUS_ALLOCATION_FAILED:
            return "allocation failed";
        case CTC_STATUS_REJECTION_LIMIT:
            return "rejection limit reached";
        case CTC_STATUS_NOT_IMPLEMENTED:
            return "not implemented";
        case CTC_STATUS_INTERNAL_ERROR:
            return "internal error";
        default:
            return "unknown status";
    }
}
