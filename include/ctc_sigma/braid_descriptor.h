#ifndef CTC_SIGMA_BRAID_DESCRIPTOR_H
#define CTC_SIGMA_BRAID_DESCRIPTOR_H

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/braid.h"
#include "ctc_sigma/status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ctc_braid_descriptor {
    uint64_t lanes[8];
    size_t token_count;
    size_t prefix_factor_count;
} ctc_braid_descriptor_t;

ctc_status_t ctc_braid_descriptor_build(
    const ctc_braid_normal_form_t *normal_form,
    uint32_t round_index,
    ctc_braid_descriptor_t *descriptor_out
);

ctc_status_t ctc_braid_descriptor_build_fixed(
    uint32_t round_index,
    ctc_braid_descriptor_t *descriptor_out
);

#ifdef __cplusplus
}
#endif

#endif
