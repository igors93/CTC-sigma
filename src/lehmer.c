#include "ctc_sigma/lehmer.h"

#include <stdbool.h>
#include <stdint.h>

#include "ctc_sigma/parameters.h"

static const uint32_t ctc_factorials[9] = {
    1U, 1U, 2U, 6U, 24U, 120U, 720U, 5040U, 40320U
};

ctc_status_t ctc_lehmer_rank(const uint8_t permutation[8], uint32_t *rank_out) {
    bool seen[8] = {false};
    uint32_t rank = 0U;

    if (permutation == NULL || rank_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        if (permutation[index] >= CTC_BRAID_STRANDS || seen[permutation[index]]) {
            return CTC_STATUS_OUT_OF_RANGE;
        }
        seen[permutation[index]] = true;
    }

    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        uint32_t smaller = 0U;
        for (uint32_t following = index + 1U; following < CTC_BRAID_STRANDS; ++following) {
            if (permutation[following] < permutation[index]) {
                ++smaller;
            }
        }
        rank += smaller * ctc_factorials[CTC_BRAID_STRANDS - 1U - index];
    }

    *rank_out = rank;
    return CTC_STATUS_OK;
}

ctc_status_t ctc_lehmer_unrank(uint32_t rank, uint8_t permutation_out[8]) {
    uint8_t available[8] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U};
    uint32_t remaining_count = CTC_BRAID_STRANDS;
    uint32_t remaining_rank = rank;

    if (permutation_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (rank >= CTC_SIMPLE_FACTOR_COUNT) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        const uint32_t factorial = ctc_factorials[CTC_BRAID_STRANDS - 1U - index];
        const uint32_t selected = factorial == 0U ? 0U : remaining_rank / factorial;
        remaining_rank = factorial == 0U ? 0U : remaining_rank % factorial;
        if (selected >= remaining_count) {
            return CTC_STATUS_INTERNAL_ERROR;
        }
        permutation_out[index] = available[selected];
        for (uint32_t move = selected; move + 1U < remaining_count; ++move) {
            available[move] = available[move + 1U];
        }
        --remaining_count;
    }
    return CTC_STATUS_OK;
}
