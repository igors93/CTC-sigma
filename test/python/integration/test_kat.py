from __future__ import annotations

import json
from pathlib import Path

import pytest

from ctc_bindings import CTC_STATUS_OK, hash256, lib, u64_array, xof

VECTOR_PATH = (
    Path(__file__).resolve().parents[3] / "test" / "vectors" / "ctc_sigma_v01_kat.json"
)


def pattern_message(length: int) -> bytes:
    return bytes(index % 256 for index in range(length))


@pytest.fixture(scope="module")
def vectors() -> dict:
    return json.loads(VECTOR_PATH.read_text())


def test_hash256_known_answers(vectors):
    for length, expected in vectors["hash256"].items():
        assert hash256(pattern_message(int(length))).hex() == expected


def test_xof_empty_message_known_answers(vectors):
    for length, expected in vectors["xof_empty_message"].items():
        assert xof(b"", int(length)).hex() == expected


def test_xof_fixed_message_known_answers(vectors):
    message = pattern_message(vectors["xof_fixed_message"]["message_length"])
    for length, expected in vectors["xof_fixed_message"]["outputs"].items():
        assert xof(message, int(length)).hex() == expected


def test_permutation_known_answers(vectors):
    for name, vector in vectors["permutation"].items():
        state = u64_array([int(value) for value in vector["input"]])
        assert lib.ctc_permutation_apply(state) == CTC_STATUS_OK, name
        assert [str(value) for value in state] == vector["output"], name
        assert lib.ctc_permutation_inverse(state) == CTC_STATUS_OK, name
        assert [str(value) for value in state] == vector["input"], name
