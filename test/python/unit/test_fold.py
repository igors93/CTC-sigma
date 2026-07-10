from __future__ import annotations

import ctypes

from ctc_bindings import (
    BraidNormalForm,
    CTC_FIELD_MODULUS,
    CTC_STATUS_OK,
    FoldTokens,
    lib,
)


def token_tag(token: int) -> int:
    return token >> 48


def token_value(token: int) -> int:
    return token & ((1 << 48) - 1)


def make_normal_form() -> BraidNormalForm:
    normal_form = BraidNormalForm()
    normal_form.infimum = -3
    normal_form.factor_count = 20
    for index in range(20):
        normal_form.factors[index] = index + 1
    return normal_form


def test_normal_form_tokenization_preserves_drop_and_keep_partition():
    normal_form = make_normal_form()
    tokens = FoldTokens()
    assert lib.ctc_fold_tokenize_normal_form(
        ctypes.byref(normal_form), 7, ctypes.byref(tokens)
    ) == CTC_STATUS_OK

    assert tokens.prefix_factor_count == 4
    assert tokens.token_count == 25
    assert [token_tag(tokens.values[index]) for index in range(5)] == [1, 2, 3, 4, 5]
    assert token_value(tokens.values[2]) == 5  # ZigZag(-3)
    assert [token_tag(tokens.values[5 + index]) for index in range(20)] == [16] * 4 + [17] * 16


def test_fold_is_deterministic_and_canonical():
    normal_form = make_normal_form()
    first = (ctypes.c_uint64 * 8)()
    second = (ctypes.c_uint64 * 8)()
    assert lib.ctc_fold_normal_form(ctypes.byref(normal_form), 7, first) == CTC_STATUS_OK
    assert lib.ctc_fold_normal_form(ctypes.byref(normal_form), 7, second) == CTC_STATUS_OK
    assert list(first) == list(second)
    assert all(0 <= value < CTC_FIELD_MODULUS for value in first)
