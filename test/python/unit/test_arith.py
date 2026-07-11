from __future__ import annotations

import ctypes
import random

from ctc_bindings import CTC_FIELD_MODULUS, CTC_STATUS_OK, lib, u64_array


def matrix_product(left, right):
    q = CTC_FIELD_MODULUS
    return [
        [sum(left[row][k] * right[k][column] for k in range(8)) % q for column in range(8)]
        for row in range(8)
    ]


def test_mds_inverse_is_a_real_inverse():
    matrix_type = (ctypes.c_uint64 * 8) * 8
    matrix = matrix_type()
    inverse = matrix_type()
    assert lib.ctc_mds_matrix(ctypes.byref(matrix)) == CTC_STATUS_OK
    assert lib.ctc_mds_inverse(ctypes.byref(inverse)) == CTC_STATUS_OK

    product = matrix_product(
        [[matrix[row][column] for column in range(8)] for row in range(8)],
        [[inverse[row][column] for column in range(8)] for row in range(8)],
    )
    assert product == [[1 if row == column else 0 for column in range(8)] for row in range(8)]


def test_arith_roundtrip():
    rng = random.Random(101)
    original = [rng.randrange(CTC_FIELD_MODULUS) for _ in range(8)]
    state = u64_array(original)

    assert lib.ctc_arith_apply(b"TEST-ARITH", 4, state, 4) == CTC_STATUS_OK
    assert list(state) != original
    assert lib.ctc_arith_inverse(b"TEST-ARITH", 4, state, 4) == CTC_STATUS_OK
    assert list(state) == original


def test_tweakable_encoder_arith_roundtrip_for_multiple_domains():
    rng = random.Random(0xA3EC02)
    for round_index in (0, 3, 7, 11):
        for block_index in (0, 1, 4, 31):
            original = [rng.randrange(CTC_FIELD_MODULUS) for _ in range(8)]
            state = u64_array(original)

            assert lib.ctc_arith_apply_encoder(
                round_index, block_index, state, 4
            ) == CTC_STATUS_OK
            assert list(state) != original
            assert lib.ctc_arith_inverse_encoder(
                round_index, block_index, state, 4
            ) == CTC_STATUS_OK
            assert list(state) == original


def test_tweakable_encoder_rejects_non_normative_indices():
    state = u64_array([0] * 8)
    assert lib.ctc_arith_apply_encoder(12, 0, state, 4) != CTC_STATUS_OK
    assert lib.ctc_arith_apply_encoder(0, 1_048_576, state, 4) != CTC_STATUS_OK
    assert lib.ctc_arith_apply_encoder(0, 0, state, 5) != CTC_STATUS_OK
