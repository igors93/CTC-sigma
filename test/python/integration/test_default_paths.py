from __future__ import annotations

import pytest

from ctc_bindings import CTC_STATUS_OK, hash256, lib, u64_array, xof


def test_default_permutation_roundtrip():
    original = [index * index + 3 for index in range(16)]
    state = u64_array(original)
    assert lib.ctc_permutation_apply(state) == CTC_STATUS_OK
    assert list(state) != original
    assert lib.ctc_permutation_inverse(state) == CTC_STATUS_OK
    assert list(state) == original


def test_default_hash_is_deterministic_and_sensitive():
    first = hash256(b"CTC-Sigma")
    second = hash256(b"CTC-Sigma")
    different = hash256(b"CTC-Sigmb")
    assert first == second
    assert first != different
    assert len(first) == 32


def test_default_xof_prefix_consistency():
    long_output = xof(b"CTC-Sigma", 64)
    short_output = xof(b"CTC-Sigma", 32)
    assert long_output[:32] == short_output
    assert len(long_output) == 64


def test_hash_and_xof_are_domain_separated():
    assert hash256(b"same input") != xof(b"same input", 32)


@pytest.mark.slow
def test_block_boundary_lengths():
    seen = set()
    for length in (0, 1, 34, 35, 36):
        message = bytes(index % 256 for index in range(length))
        digest = hash256(message)
        assert digest not in seen
        seen.add(digest)
