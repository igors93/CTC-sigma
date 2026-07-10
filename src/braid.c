#include "ctc_sigma/braid.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/lehmer.h"
#include "ctc_sigma/parameters.h"

/*
 * Permutation-braid convention (specification section 12.2):
 *
 * - A simple element of B_8 is stored as an array `map` of eight values in
 *   0..7 where `map[start_position] = end_position` of the strand.
 * - Products are read left to right: (x * y).map[s] = y.map[x.map[s]].
 * - The generator sigma_(j+1) of the specification corresponds to the
 *   transposition of positions j and j+1 for j in 0..6.
 * - The Garside element Delta corresponds to map[s] = 7 - s and has Lehmer
 *   rank 40319; the identity has Lehmer rank 0.
 *
 * With this convention:
 * - sigma_j left-divides a simple x  iff  x.map[j] > x.map[j+1];
 * - sigma_j right-divides a simple x iff  inv(x).map[j] > inv(x).map[j+1];
 * - the right complement is  d(x) = inv(x) * Delta;
 * - the flip automorphism is tau(x) = Delta^-1 * x * Delta.
 */

typedef struct ctc_braid_perm {
    uint8_t map[CTC_BRAID_STRANDS];
} ctc_braid_perm_t;

static void ctc_perm_set_identity(ctc_braid_perm_t *out) {
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        out->map[index] = (uint8_t)index;
    }
}

static int ctc_perm_is_identity(const ctc_braid_perm_t *x) {
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        if (x->map[index] != (uint8_t)index) {
            return 0;
        }
    }
    return 1;
}

static int ctc_perm_is_delta(const ctc_braid_perm_t *x) {
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        if (x->map[index] != (uint8_t)(CTC_BRAID_STRANDS - 1U - index)) {
            return 0;
        }
    }
    return 1;
}

/* out = x * y (x first, then y). Aliasing between out and inputs is allowed. */
static void ctc_perm_product(
    const ctc_braid_perm_t *x,
    const ctc_braid_perm_t *y,
    ctc_braid_perm_t *out
) {
    ctc_braid_perm_t result;
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        result.map[index] = y->map[x->map[index]];
    }
    *out = result;
}

static void ctc_perm_inverse(const ctc_braid_perm_t *x, ctc_braid_perm_t *out) {
    ctc_braid_perm_t result;
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        result.map[x->map[index]] = (uint8_t)index;
    }
    *out = result;
}

/* Right complement d(x) = inv(x) * Delta, so that x * d(x) = Delta. */
static void ctc_perm_complement(const ctc_braid_perm_t *x, ctc_braid_perm_t *out) {
    ctc_braid_perm_t inverse;
    ctc_perm_inverse(x, &inverse);
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        out->map[index] = (uint8_t)(CTC_BRAID_STRANDS - 1U - inverse.map[index]);
    }
}

/* Flip automorphism tau(x) = Delta^-1 * x * Delta. tau is an involution. */
static void ctc_perm_tau(const ctc_braid_perm_t *x, ctc_braid_perm_t *out) {
    ctc_braid_perm_t result;
    for (uint32_t index = 0U; index < CTC_BRAID_STRANDS; ++index) {
        result.map[index] = (uint8_t)(
            CTC_BRAID_STRANDS - 1U
            - x->map[CTC_BRAID_STRANDS - 1U - index]
        );
    }
    *out = result;
}

/*
 * Meet of two simples in the prefix (left-divisibility) lattice. Greedy
 * extraction is exact: whenever sigma_j left-divides both arguments it also
 * left-divides the meet, and left translation preserves the lattice order.
 */
static void ctc_perm_meet(
    const ctc_braid_perm_t *a,
    const ctc_braid_perm_t *b,
    ctc_braid_perm_t *out
) {
    ctc_braid_perm_t rest_a = *a;
    ctc_braid_perm_t rest_b = *b;
    ctc_braid_perm_t result;

    ctc_perm_set_identity(&result);
    for (;;) {
        uint32_t generator = CTC_BRAID_STRANDS;
        for (uint32_t j = 0U; j + 1U < CTC_BRAID_STRANDS; ++j) {
            if (rest_a.map[j] > rest_a.map[j + 1U]
                && rest_b.map[j] > rest_b.map[j + 1U]) {
                generator = j;
                break;
            }
        }
        if (generator == CTC_BRAID_STRANDS) {
            break;
        }

        /* result = result * sigma_j: swap values j and j+1 in the outputs. */
        for (uint32_t s = 0U; s < CTC_BRAID_STRANDS; ++s) {
            if (result.map[s] == (uint8_t)generator) {
                result.map[s] = (uint8_t)(generator + 1U);
            } else if (result.map[s] == (uint8_t)(generator + 1U)) {
                result.map[s] = (uint8_t)generator;
            }
        }
        /* Strip the common sigma_j prefix: swap entries j and j+1. */
        {
            uint8_t swap = rest_a.map[generator];
            rest_a.map[generator] = rest_a.map[generator + 1U];
            rest_a.map[generator + 1U] = swap;
            swap = rest_b.map[generator];
            rest_b.map[generator] = rest_b.map[generator + 1U];
            rest_b.map[generator + 1U] = swap;
        }
    }
    *out = result;
}

/*
 * Left-weighting of an adjacent pair: move t = meet(d(a), b) from the head
 * of b to the tail of a. Returns 1 when the pair changed.
 */
static int ctc_perm_make_left_weighted(ctc_braid_perm_t *a, ctc_braid_perm_t *b) {
    ctc_braid_perm_t complement;
    ctc_braid_perm_t transfer;
    ctc_braid_perm_t transfer_inverse;

    ctc_perm_complement(a, &complement);
    ctc_perm_meet(&complement, b, &transfer);
    if (ctc_perm_is_identity(&transfer)) {
        return 0;
    }
    ctc_perm_product(a, &transfer, a);
    ctc_perm_inverse(&transfer, &transfer_inverse);
    ctc_perm_product(&transfer_inverse, b, b);
    return 1;
}

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
    ctc_braid_perm_t simples[CTC_MAX_NORMAL_FACTORS];
    size_t simple_count = 0U;
    int64_t infimum = 0;
    size_t leading;
    ctc_status_t status;

    (void)user_context;
    if ((word == NULL && word_length != 0U) || normal_form_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (word_length > CTC_MAX_NORMAL_FACTORS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }

    /*
     * Rewrite the signed word as Delta^infimum * y_1 * ... * y_k. A negative
     * factor expands as x^-1 = Delta^-1 * tau(d(x)); pulling the Delta^-1 to
     * the front applies tau to every simple already emitted.
     */
    for (size_t index = 0U; index < word_length; ++index) {
        ctc_braid_perm_t simple;

        if ((uint32_t)word[index].simple_index >= CTC_SIMPLE_FACTOR_COUNT) {
            return CTC_STATUS_OUT_OF_RANGE;
        }
        if (word[index].sign != INT8_C(1) && word[index].sign != INT8_C(-1)) {
            return CTC_STATUS_INVALID_ARGUMENT;
        }
        status = ctc_lehmer_unrank((uint32_t)word[index].simple_index, simple.map);
        if (status != CTC_STATUS_OK) {
            return status;
        }
        if (ctc_perm_is_identity(&simple)) {
            continue;
        }

        if (word[index].sign > 0) {
            simples[simple_count] = simple;
        } else {
            for (size_t emitted = 0U; emitted < simple_count; ++emitted) {
                ctc_perm_tau(&simples[emitted], &simples[emitted]);
            }
            infimum -= 1;
            ctc_perm_complement(&simple, &simple);
            ctc_perm_tau(&simple, &simples[simple_count]);
        }
        ++simple_count;
    }

    /*
     * Iterate local left-weighting until a fixed point. Every successful
     * local move transfers positive length toward lower indices, so the
     * potential sum(i * len(x_i)) strictly decreases; the pass limit is a
     * defensive bound, not the expected iteration count.
     */
    if (simple_count > 1U) {
        const size_t max_passes = 28U * simple_count * simple_count + 4U;
        size_t pass;
        int changed = 1;

        for (pass = 0U; pass < max_passes && changed != 0; ++pass) {
            changed = 0;
            for (size_t i = 0U; i + 1U < simple_count; ++i) {
                if (ctc_perm_make_left_weighted(&simples[i], &simples[i + 1U]) != 0) {
                    changed = 1;
                }
            }
        }
        if (changed != 0) {
            return CTC_STATUS_INTERNAL_ERROR;
        }
    }

    /* Delta factors accumulate at the front and identities at the back. */
    leading = 0U;
    while (leading < simple_count && ctc_perm_is_delta(&simples[leading]) != 0) {
        ++leading;
        infimum += 1;
    }
    while (simple_count > leading
           && ctc_perm_is_identity(&simples[simple_count - 1U]) != 0) {
        --simple_count;
    }

    memset(normal_form_out, 0, sizeof(*normal_form_out));
    normal_form_out->infimum = infimum;
    normal_form_out->factor_count = simple_count - leading;
    for (size_t index = leading; index < simple_count; ++index) {
        uint32_t rank = 0U;

        if (ctc_perm_is_identity(&simples[index]) != 0
            || ctc_perm_is_delta(&simples[index]) != 0) {
            return CTC_STATUS_INTERNAL_ERROR;
        }
        status = ctc_lehmer_rank(simples[index].map, &rank);
        if (status != CTC_STATUS_OK) {
            return status;
        }
        normal_form_out->factors[index - leading] = (uint16_t)rank;
    }
    return CTC_STATUS_OK;
}
