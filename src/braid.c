#include "ctc_sigma/braid.h"

#include <stddef.h>
#include <stdint.h>

#include "ctc_sigma/lehmer.h"
#include "ctc_sigma/parameters.h"

ctc_status_t ctc_braid_simple_permutation(
    uint16_t simple_index,
    uint8_t permutation_out[8]
) {
    if (permutation_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if ((uint32_t)simple_index >= CTC_SIMPLE_FACTOR_COUNT) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    return ctc_lehmer_unrank((uint32_t)simple_index, permutation_out);
}

ctc_status_t ctc_braid_normalize_left(
    const ctc_signed_factor_t *word,
    size_t word_length,
    ctc_braid_normal_form_t *normal_form_out,
    void *user_context
) {
    (void)user_context;
    if ((word == NULL && word_length != 0U) || normal_form_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    /*
     * The exact B_8 left Garside normal form is the only intentionally open
     * mathematical module in this first repository version. Returning an
     * explicit status prevents accidental use of a non-equivalent reduction.
     */
    return CTC_STATUS_NOT_IMPLEMENTED;
}
