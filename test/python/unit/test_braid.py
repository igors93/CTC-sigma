from __future__ import annotations

import ctypes
import random

import garside_reference as reference
from ctc_bindings import (
    BraidNormalForm,
    CTC_STATUS_OK,
    SignedFactor,
    lib,
)

DELTA_RANK = 40319


def c_normalize(word: list[tuple[int, int]]) -> tuple[int, list[int]]:
    factors = (SignedFactor * max(len(word), 1))()
    for index, (rank, sign) in enumerate(word):
        factors[index].simple_index = rank
        factors[index].sign = sign
    normal_form = BraidNormalForm()
    status = lib.ctc_braid_normalize_left(
        factors, len(word), ctypes.byref(normal_form), None
    )
    assert status == CTC_STATUS_OK, f"unexpected status {status}"
    return normal_form.infimum, [
        normal_form.factors[index] for index in range(normal_form.factor_count)
    ]


def test_empty_word_is_identity():
    assert c_normalize([]) == (0, [])


def test_identity_factors_are_absorbed():
    assert c_normalize([(0, 1), (0, -1)]) == (0, [])


def test_positive_delta_increases_infimum():
    assert c_normalize([(DELTA_RANK, 1)]) == (1, [])
    assert c_normalize([(DELTA_RANK, 1), (DELTA_RANK, 1)]) == (2, [])


def test_negative_delta_decreases_infimum():
    assert c_normalize([(DELTA_RANK, -1)]) == (-1, [])


def test_single_proper_factor_is_already_normal():
    for rank in (1, 2, 40318, 12345):
        infimum, factors = c_normalize([(rank, 1)])
        assert infimum == 0
        assert factors == [rank]


def test_factor_times_inverse_cancels():
    for rank in (1, 7, 40318, 20000):
        assert c_normalize([(rank, 1), (rank, -1)]) == (0, [])
        assert c_normalize([(rank, -1), (rank, 1)]) == (0, [])


def test_factor_and_complement_compose_to_delta():
    for rank in (1, 9, 31337):
        simple = reference.lehmer_unrank(rank)
        complement_rank = reference.lehmer_rank(reference.complement(simple))
        assert c_normalize([(rank, 1), (complement_rank, 1)]) == (1, [])


def test_output_factors_are_proper_and_left_weighted():
    rng = random.Random(0xC7C51)
    for _ in range(50):
        word = [
            (rng.randrange(1, 40320), rng.choice((1, -1)))
            for _ in range(rng.randrange(1, 33))
        ]
        _, factors = c_normalize(word)
        for rank in factors:
            assert 1 <= rank <= 40318
        for left_rank, right_rank in zip(factors, factors[1:]):
            left = reference.lehmer_unrank(left_rank)
            right = reference.lehmer_unrank(right_rank)
            assert reference.starting_set(right) <= reference.finishing_set(left)


def test_matches_independent_python_reference_on_random_words():
    rng = random.Random(0x5D9A17)
    for _ in range(200):
        word = [
            (rng.randrange(0, 40320), rng.choice((1, -1)))
            for _ in range(rng.randrange(0, 40))
        ]
        assert c_normalize(word) == reference.normalize(word)


def test_normal_form_projects_to_the_word_permutation():
    rng = random.Random(0xB8B8)
    for _ in range(100):
        word = [
            (rng.randrange(1, 40320), rng.choice((1, -1)))
            for _ in range(rng.randrange(1, 33))
        ]
        infimum, factors = c_normalize(word)
        assert reference.normal_form_permutation(
            infimum, factors
        ) == reference.word_permutation(word)


def test_rejects_out_of_range_index_and_invalid_sign():
    factors = (SignedFactor * 1)()
    normal_form = BraidNormalForm()

    factors[0].simple_index = 40320
    factors[0].sign = 1
    assert (
        lib.ctc_braid_normalize_left(factors, 1, ctypes.byref(normal_form), None)
        != CTC_STATUS_OK
    )

    factors[0].simple_index = 5
    factors[0].sign = 0
    assert (
        lib.ctc_braid_normalize_left(factors, 1, ctypes.byref(normal_form), None)
        != CTC_STATUS_OK
    )
