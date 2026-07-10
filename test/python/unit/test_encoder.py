from __future__ import annotations

import ctypes

from ctc_bindings import (
    CTC_FACTORS_PER_BRANCH,
    CTC_STATUS_OK,
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


def test_encoder_is_deterministic_and_respects_factor_ranges():
    first = generate_once()
    second = generate_once()
    assert first == second
    status, factors, block_count = first
    assert status == CTC_STATUS_OK
    assert block_count >= 4
    assert len(factors) == CTC_FACTORS_PER_BRANCH
    assert all(1 <= index <= 40319 for index, _ in factors)
    assert all(sign in (-1, 1) for _, sign in factors)
