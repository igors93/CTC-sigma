#ifndef CTC_SIGMA_FIELD_H
#define CTC_SIGMA_FIELD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t ctc_field_reduce_u64(uint64_t value);
uint64_t ctc_field_add(uint64_t left, uint64_t right);
uint64_t ctc_field_sub(uint64_t left, uint64_t right);
uint64_t ctc_field_mul(uint64_t left, uint64_t right);
uint64_t ctc_field_pow(uint64_t base, uint64_t exponent);
uint64_t ctc_field_inv(uint64_t value);
uint64_t ctc_field_dickson(uint64_t value, uint64_t degree);

#ifdef __cplusplus
}
#endif

#endif
