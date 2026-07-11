from __future__ import annotations

import ctypes
import hashlib
import json
from pathlib import Path

import pytest

from ctc_bindings import (
    CTC_ENCODER_CONSTANT_RC,
    CTC_ENCODER_CONSTANT_SBOX_A,
    CTC_ENCODER_CONSTANT_SBOX_B,
    CTC_ENCODER_CONSTANT_SBOX_C,
    CTC_FACTORS_PER_BRANCH,
    CTC_STATUS_OK,
    SignedFactor,
    hash256,
    lib,
    u64_array,
    xof,
)

VECTOR_DIRECTORY = Path(__file__).resolve().parents[3] / "test" / "vectors"
VECTOR_PATH = VECTOR_DIRECTORY / "ctc_sigma_v02_kat.json"
MANIFEST_PATH = VECTOR_DIRECTORY / "ctc_sigma_v02_encoder_constants.json"
MANIFEST_HASH_PATH = VECTOR_DIRECTORY / "ctc_sigma_v02_encoder_constants.sha256"
COMPONENT_IDS = {
    "RC": CTC_ENCODER_CONSTANT_RC,
    "SBOX-A": CTC_ENCODER_CONSTANT_SBOX_A,
    "SBOX-B": CTC_ENCODER_CONSTANT_SBOX_B,
    "SBOX-C": CTC_ENCODER_CONSTANT_SBOX_C,
}


def derive_manifest_value_python(record: dict) -> int:
    component = record["component"].encode("ascii")
    seed = (
        b"CTC-SIGMA-v0.2|A_ENC-TWEAK|"
        + bytes([len(component)])
        + component
        + record["round"].to_bytes(4, "little")
        + record["block"].to_bytes(4, "little")
        + record["subround"].to_bytes(4, "little")
        + record["lane"].to_bytes(4, "little")
    )
    value = int.from_bytes(hashlib.shake_256(seed).digest(16), "little") % (2**61 - 1)
    return 1 if record["component"] == "SBOX-B" and value == 0 else value


def pattern_message(length: int) -> bytes:
    return bytes(index % 256 for index in range(length))


@pytest.fixture(scope="module")
def vectors() -> dict:
    return json.loads(VECTOR_PATH.read_text())


def test_vector_version(vectors):
    assert vectors["version"] == "CTC-Sigma v0.2"


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


def test_encoder_block_known_answers(vectors):
    source = u64_array([int(value) for value in vectors["encoder"]["input"]])
    for round_index, blocks in vectors["encoder"]["blocks"].items():
        for block_index, expected in blocks.items():
            output = (ctypes.c_uint64 * 8)()
            assert lib.ctc_encoder_generate_block(
                source, int(round_index), int(block_index), output
            ) == CTC_STATUS_OK
            assert [str(value) for value in output] == expected


def test_encoder_factor_stream_known_answer(vectors):
    source = u64_array([int(value) for value in vectors["encoder"]["input"]])
    factor_vector = vectors["encoder"]["factor_stream"]
    factors = (SignedFactor * CTC_FACTORS_PER_BRANCH)()
    block_count = ctypes.c_size_t()
    assert lib.ctc_encoder_generate_factors(
        source, factor_vector["round"], factors, ctypes.byref(block_count)
    ) == CTC_STATUS_OK
    assert block_count.value == factor_vector["block_count"]
    assert [
        {"simple_index": factor.simple_index, "sign": factor.sign}
        for factor in factors
    ] == factor_vector["factors"]


def test_encoder_constant_manifest_hash_and_values():
    manifest_bytes = MANIFEST_PATH.read_bytes()
    expected_hash, expected_name = MANIFEST_HASH_PATH.read_text().split()
    assert expected_name == MANIFEST_PATH.name
    assert hashlib.sha256(manifest_bytes).hexdigest() == expected_hash

    manifest = json.loads(manifest_bytes)
    assert manifest["record_count"] == len(manifest["records"])
    for record in manifest["records"]:
        value = ctypes.c_uint64()
        assert lib.ctc_encoder_constant_derive(
            COMPONENT_IDS[record["component"]],
            record["round"],
            record["block"],
            record["subround"],
            record["lane"],
            ctypes.byref(value),
        ) == CTC_STATUS_OK
        assert str(value.value) == record["value"]
        assert str(derive_manifest_value_python(record)) == record["value"]
