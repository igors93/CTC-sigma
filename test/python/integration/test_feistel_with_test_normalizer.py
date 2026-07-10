from __future__ import annotations

import ctypes

import pytest

from ctc_bindings import (
    BraidNormalForm,
    CTC_STATUS_OK,
    NORMALIZER_CALLBACK,
    lib,
    u64_array,
)


@NORMALIZER_CALLBACK
def injected_normalizer(word, word_length, normal_form, _context):
    """Test adapter only: it is not a Garside implementation and is never linked into C."""
    target: BraidNormalForm = normal_form.contents
    target.infimum = 0
    target.factor_count = word_length
    for index in range(word_length):
        target.factors[index] = word[index].simple_index
    return CTC_STATUS_OK


@pytest.mark.slow
def test_feistel_roundtrip_with_injected_test_normalizer():
    original = [index * index + 3 for index in range(16)]
    state = u64_array(original)
    assert lib.ctc_permutation_apply_with_normalizer(
        state, injected_normalizer, None
    ) == CTC_STATUS_OK
    assert list(state) != original
    assert lib.ctc_permutation_inverse_with_normalizer(
        state, injected_normalizer, None
    ) == CTC_STATUS_OK
    assert list(state) == original
