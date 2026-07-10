from __future__ import annotations

import random

from ctc_bindings import CTC_FIELD_MODULUS, lib


def dickson_reference(value: int, degree: int) -> int:
    q = CTC_FIELD_MODULUS
    if degree == 0:
        return 2
    if degree == 1:
        return value % q
    previous, current = 2, value % q
    for _ in range(2, degree + 1):
        previous, current = current, (value * current - previous) % q
    return current


def test_field_operations_match_python_modular_arithmetic():
    rng = random.Random(20260710)
    q = CTC_FIELD_MODULUS
    for _ in range(200):
        left = rng.randrange(q)
        right = rng.randrange(q)
        assert lib.ctc_field_add(left, right) == (left + right) % q
        assert lib.ctc_field_sub(left, right) == (left - right) % q
        assert lib.ctc_field_mul(left, right) == (left * right) % q


def test_field_inverse_follows_specification():
    q = CTC_FIELD_MODULUS
    assert lib.ctc_field_inv(0) == 0
    for value in [1, 2, 3, 17, q - 1, 123456789012345]:
        inverse = lib.ctc_field_inv(value)
        assert (value * inverse) % q == 1


def test_dickson_degrees_match_reference_recurrence():
    for value in [0, 1, 2, 17, CTC_FIELD_MODULUS - 1]:
        for degree in [0, 1, 23, 47]:
            assert lib.ctc_field_dickson(value, degree) == dickson_reference(value, degree)
