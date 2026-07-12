from __future__ import annotations

import ctypes
import random

from ctc_bindings import (
    CTC_FACTORS_PER_BRANCH,
    CTC_FIELD_MODULUS,
    CTC_STATUS_OK,
    CTC_STATUS_OUT_OF_RANGE,
    SignedFactor,
    lib,
    u64_array,
)


def generate_once():
    mixed_input = u64_array([0, 1, 2, 3, 4, 5, 6, 7])
    factors = (SignedFactor * CTC_FACTORS_PER_BRANCH)()
    blocks = ctypes.c_size_t()
    status = lib.ctc_encoder_generate_factors(
        mixed_input,
        2,
        factors,
        ctypes.byref(blocks),
    )
    return status, [(item.simple_index, item.sign) for item in factors], blocks.value


def generate_block(values: list[int], round_index: int, block_index: int) -> list[int]:
    source = u64_array(values)
    output = (ctypes.c_uint64 * 8)()
    assert lib.ctc_encoder_generate_block(
        source,
        round_index,
        block_index,
        output,
    ) == CTC_STATUS_OK
    return list(output)


def test_encoder_is_deterministic_and_respects_factor_ranges():
    first = generate_once()
    second = generate_once()
    assert first == second
    status, factors, block_count = first
    assert status == CTC_STATUS_OK
    assert block_count >= 3
    assert len(factors) == CTC_FACTORS_PER_BRANCH
    assert all(1 <= index <= 40319 for index, _ in factors)
    assert all(sign in (-1, 1) for _, sign in factors)


def test_encoder_block_generation_supports_input_output_aliasing():
    state = u64_array([11, 22, 33, 44, 55, 66, 77, 88])
    separate = generate_block(list(state), round_index=5, block_index=3)
    assert lib.ctc_encoder_generate_block(state, 5, 3, state) == CTC_STATUS_OK
    assert list(state) == separate


def test_v01_input_counter_alias_is_not_present_in_v03():
    rng = random.Random(0xC7C502)

    # The old construction satisfied this equality for every x. Testing every
    # production round and several adjacent blocks makes accidental regression
    # to `x[0] += block_index` immediately visible.
    for round_index in range(12):
        for block_index in range(4):
            for _ in range(4):
                base = [rng.randrange(CTC_FIELD_MODULUS) for _ in range(8)]
                related = base.copy()
                related[0] = (related[0] + 1) % CTC_FIELD_MODULUS

                assert generate_block(
                    related, round_index, block_index
                ) != generate_block(
                    base, round_index, block_index + 1
                )


def test_block_zero_is_tweaked_and_blocks_do_not_reuse_the_same_permutation():
    source = [0, 1, 2, 3, 4, 5, 6, 7]
    outputs = {
        tuple(generate_block(source, round_index=7, block_index=block_index))
        for block_index in range(5)
    }
    assert len(outputs) == 5


def test_encoder_rejects_out_of_range_round_and_block():
    source = u64_array([0] * 8)
    output = (ctypes.c_uint64 * 8)()
    assert lib.ctc_encoder_generate_block(source, 12, 0, output) == CTC_STATUS_OUT_OF_RANGE
    assert lib.ctc_encoder_generate_block(
        source, 0, 1_048_576, output
    ) == CTC_STATUS_OUT_OF_RANGE
