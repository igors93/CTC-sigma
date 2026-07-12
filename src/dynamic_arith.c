#include "ctc_sigma/dynamic_arith.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "ctc_sigma/arith.h"
#include "ctc_sigma/constants.h"
#include "ctc_sigma/field.h"
#include "ctc_sigma/lehmer.h"
#include "ctc_sigma/parameters.h"
#include "ctc_sigma/validation.h"

static ctc_status_t ctc_descriptor_permutation(
    const ctc_braid_descriptor_t *descriptor,
    uint8_t permutation_out[8]
) {
    const uint64_t threshold =
        (CTC_FIELD_MODULUS / CTC_SIMPLE_FACTOR_COUNT) * CTC_SIMPLE_FACTOR_COUNT;

    if (descriptor == NULL || permutation_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }

    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        if (descriptor->lanes[lane] < threshold) {
            const uint32_t rank = (uint32_t)(
                descriptor->lanes[lane] % CTC_SIMPLE_FACTOR_COUNT
            );
            return ctc_lehmer_unrank(rank, permutation_out);
        }
    }
    return CTC_STATUS_REJECTION_LIMIT;
}

static ctc_status_t ctc_validate_permutation(const uint8_t permutation[8]) {
    uint8_t seen[8] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

    if (permutation == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    for (uint32_t index = 0U; index < CTC_BRANCH_LANES; ++index) {
        if (permutation[index] >= CTC_BRANCH_LANES
            || seen[permutation[index]] != 0U) {
            return CTC_STATUS_OUT_OF_RANGE;
        }
        seen[permutation[index]] = 1U;
    }
    return CTC_STATUS_OK;
}

static uint64_t ctc_descriptor_mix(
    const ctc_braid_descriptor_t *descriptor,
    uint32_t subround,
    uint32_t lane
) {
    const uint32_t index_a = lane;
    const uint32_t index_b = (lane + subround + 1U) % CTC_BRANCH_LANES;
    const uint32_t index_c = (lane + 2U * subround + 3U) % CTC_BRANCH_LANES;
    const uint64_t square = ctc_field_mul(
        descriptor->lanes[index_c],
        descriptor->lanes[index_c]
    );

    return ctc_field_add(
        ctc_field_add(
            descriptor->lanes[index_a],
            ctc_field_mul((uint64_t)(subround + 2U), descriptor->lanes[index_b])
        ),
        ctc_field_mul((uint64_t)(lane + 3U), square)
    );
}

ctc_status_t ctc_dynamic_arith_config_build(
    const ctc_braid_descriptor_t *descriptor,
    uint32_t round_index,
    ctc_dynamic_arith_config_t *config_out
) {
    ctc_dynamic_arith_config_t config;
    ctc_status_t status;

    if (descriptor == NULL || config_out == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    if (round_index >= CTC_FEISTEL_ROUNDS) {
        return CTC_STATUS_OUT_OF_RANGE;
    }
    status = ctc_validate_canonical_lanes(descriptor->lanes, CTC_BRANCH_LANES);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memset(&config, 0, sizeof(config));
    status = ctc_descriptor_permutation(descriptor, config.lane_permutation);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    const uint8_t start_with_47 = (uint8_t)(
        (descriptor->lanes[7] + (uint64_t)round_index) & UINT64_C(1)
    );
    for (uint32_t subround = 0U;
         subround < CTC_ARITH_DYNAMIC_SUBROUNDS;
         ++subround) {
        uint64_t base_rc[CTC_BRANCH_LANES];
        uint64_t base_a[CTC_BRANCH_LANES];
        uint64_t base_b[CTC_BRANCH_LANES];
        uint64_t base_c[CTC_BRANCH_LANES];

        config.degrees[subround] = ((subround & 1U) == start_with_47)
            ? UINT8_C(47)
            : UINT8_C(23);

        status = ctc_v03_constant_derive_lanes(
            "DYNAMIC-POST",
            CTC_V03_CONSTANT_DYNAMIC_RC,
            round_index,
            subround,
            0U,
            base_rc
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_v03_constant_derive_lanes(
            "DYNAMIC-POST",
            CTC_V03_CONSTANT_DYNAMIC_A,
            round_index,
            subround,
            0U,
            base_a
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_v03_constant_derive_lanes(
            "DYNAMIC-POST",
            CTC_V03_CONSTANT_DYNAMIC_B,
            round_index,
            subround,
            0U,
            base_b
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_v03_constant_derive_lanes(
            "DYNAMIC-POST",
            CTC_V03_CONSTANT_DYNAMIC_C,
            round_index,
            subround,
            0U,
            base_c
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }

        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            const uint32_t neighbor =
                (lane + subround + 2U) % CTC_BRANCH_LANES;
            const uint64_t mix = ctc_descriptor_mix(descriptor, subround, lane);

            config.round_constants[subround][lane] = ctc_field_add(base_rc[lane], mix);
            config.sbox_a[subround][lane] = ctc_field_add(
                base_a[lane],
                descriptor->lanes[neighbor]
            );
            config.sbox_b[subround][lane] = ctc_field_add(
                ctc_field_add(base_b[lane], mix),
                UINT64_C(1)
            );
            if (config.sbox_b[subround][lane] == 0U) {
                config.sbox_b[subround][lane] = 1U;
            }
            config.sbox_c[subround][lane] = ctc_field_add(
                base_c[lane],
                ctc_field_add(descriptor->lanes[lane], mix)
            );
        }
    }

    *config_out = config;
    return CTC_STATUS_OK;
}

ctc_status_t ctc_dynamic_arith_inject_descriptor(
    const uint64_t state[8],
    const ctc_braid_descriptor_t *descriptor,
    uint64_t output[8]
) {
    uint64_t working[CTC_BRANCH_LANES];
    ctc_status_t status;

    if (state == NULL || descriptor == NULL || output == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    status = ctc_validate_canonical_lanes(state, CTC_BRANCH_LANES);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_validate_canonical_lanes(descriptor->lanes, CTC_BRANCH_LANES);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
        const uint32_t next = (lane + 3U) % CTC_BRANCH_LANES;
        const uint32_t far = (lane + 5U) % CTC_BRANCH_LANES;
        working[lane] = ctc_field_add(
            state[lane],
            ctc_field_add(
                descriptor->lanes[lane],
                ctc_field_add(
                    ctc_field_mul((uint64_t)(lane + 2U), descriptor->lanes[next]),
                    ctc_field_mul(descriptor->lanes[far], descriptor->lanes[far])
                )
            )
        );
    }
    memcpy(output, working, sizeof(working));
    return CTC_STATUS_OK;
}

static uint64_t ctc_dynamic_sbox(
    uint64_t value,
    uint8_t degree,
    uint64_t constant_a,
    uint64_t constant_b,
    uint64_t constant_c
) {
    uint64_t transformed = ctc_field_add(value, constant_c);

    transformed = ctc_field_dickson(transformed, degree);
    transformed = ctc_field_inv(transformed);
    transformed = ctc_field_mul(constant_b, transformed);
    return ctc_field_add(constant_a, transformed);
}

ctc_status_t ctc_dynamic_arith_apply(
    uint64_t state[8],
    const ctc_dynamic_arith_config_t *config
) {
    uint64_t base_matrix[8][8];
    uint64_t working[CTC_BRANCH_LANES];
    ctc_status_t status;

    if (state == NULL || config == NULL) {
        return CTC_STATUS_INVALID_ARGUMENT;
    }
    status = ctc_validate_canonical_lanes(state, CTC_BRANCH_LANES);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    status = ctc_validate_permutation(config->lane_permutation);
    if (status != CTC_STATUS_OK) {
        return status;
    }
    for (uint32_t subround = 0U;
         subround < CTC_ARITH_DYNAMIC_SUBROUNDS;
         ++subround) {
        status = ctc_validate_canonical_lanes(
            config->round_constants[subround],
            CTC_BRANCH_LANES
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_validate_canonical_lanes(
            config->sbox_a[subround],
            CTC_BRANCH_LANES
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_validate_canonical_lanes(
            config->sbox_b[subround],
            CTC_BRANCH_LANES
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        status = ctc_validate_canonical_lanes(
            config->sbox_c[subround],
            CTC_BRANCH_LANES
        );
        if (status != CTC_STATUS_OK) {
            return status;
        }
        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            if (config->sbox_b[subround][lane] == 0U) {
                return CTC_STATUS_OUT_OF_RANGE;
            }
        }
    }
    status = ctc_mds_matrix(base_matrix);
    if (status != CTC_STATUS_OK) {
        return status;
    }

    memcpy(working, state, sizeof(working));
    for (uint32_t subround = 0U;
         subround < CTC_ARITH_DYNAMIC_SUBROUNDS;
         ++subround) {
        uint64_t nonlinear[CTC_BRANCH_LANES];
        uint64_t mixed[CTC_BRANCH_LANES];

        if (config->degrees[subround] != 23U
            && config->degrees[subround] != 47U) {
            return CTC_STATUS_OUT_OF_RANGE;
        }
        for (uint32_t lane = 0U; lane < CTC_BRANCH_LANES; ++lane) {
            if (config->sbox_b[subround][lane] == 0U) {
                return CTC_STATUS_OUT_OF_RANGE;
            }
            nonlinear[lane] = ctc_dynamic_sbox(
                ctc_field_add(
                    working[lane],
                    config->round_constants[subround][lane]
                ),
                config->degrees[subround],
                config->sbox_a[subround][lane],
                config->sbox_b[subround][lane],
                config->sbox_c[subround][lane]
            );
        }

        for (uint32_t row = 0U; row < CTC_BRANCH_LANES; ++row) {
            uint64_t accumulator = 0U;
            const uint32_t mapped_row = config->lane_permutation[
                (row + subround) % CTC_BRANCH_LANES
            ];
            for (uint32_t column = 0U; column < CTC_BRANCH_LANES; ++column) {
                const uint32_t mapped_column = config->lane_permutation[
                    (column + subround) % CTC_BRANCH_LANES
                ];
                accumulator = ctc_field_add(
                    accumulator,
                    ctc_field_mul(
                        base_matrix[mapped_row][mapped_column],
                        nonlinear[column]
                    )
                );
            }
            mixed[row] = accumulator;
        }
        memcpy(working, mixed, sizeof(working));
    }

    memcpy(state, working, sizeof(working));
    return CTC_STATUS_OK;
}
