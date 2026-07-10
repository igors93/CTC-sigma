#ifndef CTC_SIGMA_STATUS_H
#define CTC_SIGMA_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ctc_status {
    CTC_STATUS_OK = 0,
    CTC_STATUS_INVALID_ARGUMENT = 1,
    CTC_STATUS_OUT_OF_RANGE = 2,
    CTC_STATUS_BUFFER_TOO_SMALL = 3,
    CTC_STATUS_ALLOCATION_FAILED = 4,
    CTC_STATUS_REJECTION_LIMIT = 5,
    CTC_STATUS_NOT_IMPLEMENTED = 6,
    CTC_STATUS_INTERNAL_ERROR = 7
} ctc_status_t;

const char *ctc_status_string(ctc_status_t status);

#ifdef __cplusplus
}
#endif

#endif
