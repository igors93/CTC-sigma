#ifndef CTC_SIGMA_LEHMER_H
#define CTC_SIGMA_LEHMER_H

#include <stdint.h>

#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

ctc_status_t ctc_lehmer_rank(const uint8_t permutation[8], uint32_t *rank_out);
ctc_status_t ctc_lehmer_unrank(uint32_t rank, uint8_t permutation_out[8]);

#ifdef __cplusplus
}
#endif

#endif
