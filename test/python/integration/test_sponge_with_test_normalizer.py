from __future__ import annotations

import ctypes

import pytest

from ctc_bindings import (
    BraidNormalForm,
    CTC_STATUS_OK,
    NORMALIZER_CALLBACK,
    lib,
)


@NORMALIZER_CALLBACK
def injected_normalizer(word, word_length, normal_form, _context):
    """Architecture adapter only; this is not the B_8 Garside normal form."""
    target: BraidNormalForm = normal_form.contents
    target.infimum = 0
    target.factor_count = word_length
    for index in range(word_length):
        target.factors[index] = word[index].simple_index
    return CTC_STATUS_OK


def input_buffer(message: bytes):
    return (ctypes.c_uint8 * len(message)).from_buffer_copy(message) if message else None


@pytest.mark.slow
def test_hash_and_xof_paths_are_deterministic_and_domain_separated():
    message = b"CTC-Sigma initial integration test"
    source = input_buffer(message)
    digest_first = (ctypes.c_uint8 * 32)()
    digest_second = (ctypes.c_uint8 * 32)()
    xof_output = (ctypes.c_uint8 * 64)()

    assert lib.ctc_hash256_with_normalizer(
        source, len(message), injected_normalizer, None, digest_first
    ) == CTC_STATUS_OK
    assert lib.ctc_hash256_with_normalizer(
        source, len(message), injected_normalizer, None, digest_second
    ) == CTC_STATUS_OK
    assert lib.ctc_xof_with_normalizer(
        source, len(message), injected_normalizer, None, xof_output, len(xof_output)
    ) == CTC_STATUS_OK

    assert bytes(digest_first) == bytes(digest_second)
    assert bytes(digest_first) != bytes(xof_output[:32])
    assert any(digest_first)
    assert any(xof_output)
