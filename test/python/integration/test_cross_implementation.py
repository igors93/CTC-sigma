from __future__ import annotations

import ctypes

import pytest

import garside_reference as reference
from ctc_bindings import (
    BraidNormalForm,
    CTC_STATUS_OK,
    NORMALIZER_CALLBACK,
    hash256,
    lib,
    xof,
)


@NORMALIZER_CALLBACK
def reference_normalizer(word, word_length, normal_form, _context):
    """Independent Python Garside normalizer driving the C pipeline."""
    signed_word = [
        (word[index].simple_index, word[index].sign) for index in range(word_length)
    ]
    infimum, factors = reference.normalize(signed_word)
    target: BraidNormalForm = normal_form.contents
    target.infimum = infimum
    target.factor_count = len(factors)
    for index, rank in enumerate(factors):
        target.factors[index] = rank
    return CTC_STATUS_OK


def input_buffer(message: bytes):
    return (
        (ctypes.c_uint8 * len(message)).from_buffer_copy(message)
        if message
        else None
    )


@pytest.mark.slow
def test_hash256_agrees_between_c_and_python_normalizers():
    for message in (b"", b"a", b"CTC-Sigma cross-implementation vector"):
        digest_python = (ctypes.c_uint8 * 32)()
        assert lib.ctc_hash256_with_normalizer(
            input_buffer(message),
            len(message),
            reference_normalizer,
            None,
            digest_python,
        ) == CTC_STATUS_OK
        assert bytes(digest_python) == hash256(message)


@pytest.mark.slow
def test_xof_agrees_between_c_and_python_normalizers():
    message = b"CTC-Sigma XOF cross-check"
    output_python = (ctypes.c_uint8 * 96)()
    assert lib.ctc_xof_with_normalizer(
        input_buffer(message),
        len(message),
        reference_normalizer,
        None,
        output_python,
        len(output_python),
    ) == CTC_STATUS_OK
    assert bytes(output_python) == xof(message, 96)
