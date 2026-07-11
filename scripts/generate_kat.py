#!/usr/bin/env python3
"""Generate frozen known-answer vectors for CTC-Sigma v0.2.

Message convention: a message of length n consists of bytes 0, 1, ...,
(n-1) modulo 256. The fixed XOF message is the 41-byte instance of the same
pattern.

The script also freezes a representative A_ENC constant manifest for rounds
0, 3, 7, and 11 and blocks 0 through 4, then writes its SHA-256 checksum.
Run from the repository root after building the shared library:

    python scripts/generate_kat.py
"""

from __future__ import annotations

import ctypes
import hashlib
import json
import os
import sys
from pathlib import Path

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPOSITORY_ROOT / "test" / "python"))
os.environ.setdefault(
    "CTC_SIGMA_LIBRARY", str(REPOSITORY_ROOT / "build" / "libctc_sigma.so")
)

from ctc_bindings import (  # noqa: E402
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

HASH_MESSAGE_LENGTHS = [0, 1, 39, 40, 41, 80, 1024]
XOF_OUTPUT_LENGTHS = [1, 31, 32, 40, 41, 64, 1000]
FIXED_XOF_MESSAGE_LENGTH = 41
ENCODER_VECTOR_INPUT = list(range(8))
ENCODER_VECTOR_ROUNDS = [0, 3, 7, 11]
ENCODER_VECTOR_BLOCKS = [0, 1, 2, 3, 4]
ENCODER_COMPONENTS = [
    ("RC", CTC_ENCODER_CONSTANT_RC),
    ("SBOX-A", CTC_ENCODER_CONSTANT_SBOX_A),
    ("SBOX-B", CTC_ENCODER_CONSTANT_SBOX_B),
    ("SBOX-C", CTC_ENCODER_CONSTANT_SBOX_C),
]


def pattern_message(length: int) -> bytes:
    return bytes(index % 256 for index in range(length))


def permutation_vector(state_values: list[int]) -> dict:
    state = u64_array(state_values)
    assert lib.ctc_permutation_apply(state) == CTC_STATUS_OK
    output = [str(value) for value in state]
    assert lib.ctc_permutation_inverse(state) == CTC_STATUS_OK
    assert list(state) == state_values
    return {
        "input": [str(value) for value in state_values],
        "output": output,
    }


def encoder_block_vector(round_index: int, block_index: int) -> list[str]:
    source = u64_array(ENCODER_VECTOR_INPUT)
    output = (ctypes.c_uint64 * 8)()
    assert lib.ctc_encoder_generate_block(
        source, round_index, block_index, output
    ) == CTC_STATUS_OK
    return [str(value) for value in output]


def encoder_factor_vector() -> dict:
    source = u64_array(ENCODER_VECTOR_INPUT)
    factors = (SignedFactor * CTC_FACTORS_PER_BRANCH)()
    block_count = ctypes.c_size_t()
    assert lib.ctc_encoder_generate_factors(
        source, 2, factors, ctypes.byref(block_count)
    ) == CTC_STATUS_OK
    return {
        "round": 2,
        "block_count": block_count.value,
        "factors": [
            {
                "simple_index": factors[index].simple_index,
                "sign": factors[index].sign,
            }
            for index in range(CTC_FACTORS_PER_BRANCH)
        ],
    }


def encoder_constant(
    component: int,
    round_index: int,
    block_index: int,
    subround_index: int,
    lane_index: int,
) -> int:
    value = ctypes.c_uint64()
    assert lib.ctc_encoder_constant_derive(
        component,
        round_index,
        block_index,
        subround_index,
        lane_index,
        ctypes.byref(value),
    ) == CTC_STATUS_OK
    return value.value


def build_encoder_constant_manifest() -> dict:
    records = []
    for component_name, component in ENCODER_COMPONENTS:
        for round_index in ENCODER_VECTOR_ROUNDS:
            for block_index in ENCODER_VECTOR_BLOCKS:
                for subround_index in range(4):
                    for lane_index in range(8):
                        records.append(
                            {
                                "component": component_name,
                                "round": round_index,
                                "block": block_index,
                                "subround": subround_index,
                                "lane": lane_index,
                                "value": str(
                                    encoder_constant(
                                        component,
                                        round_index,
                                        block_index,
                                        subround_index,
                                        lane_index,
                                    )
                                ),
                            }
                        )
    return {
        "version": "CTC-Sigma v0.2",
        "domain": "CTC-SIGMA-v0.2|A_ENC-TWEAK|",
        "seed_format": (
            "prefix || LEN8(component) || component || LE32(round) || "
            "LE32(block) || LE32(subround) || LE32(lane)"
        ),
        "rounds": ENCODER_VECTOR_ROUNDS,
        "blocks": ENCODER_VECTOR_BLOCKS,
        "record_count": len(records),
        "records": records,
    }


def write_json(path: Path, payload: dict) -> bytes:
    encoded = (json.dumps(payload, indent=2) + "\n").encode("utf-8")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(encoded)
    return encoded


def main() -> None:
    vectors = {
        "version": "CTC-Sigma v0.2",
        "message_convention": "byte[i] = i mod 256",
        "hash256": {
            str(length): hash256(pattern_message(length)).hex()
            for length in HASH_MESSAGE_LENGTHS
        },
        "xof_empty_message": {
            str(length): xof(b"", length).hex() for length in XOF_OUTPUT_LENGTHS
        },
        "xof_fixed_message": {
            "message_length": FIXED_XOF_MESSAGE_LENGTH,
            "outputs": {
                str(length): xof(
                    pattern_message(FIXED_XOF_MESSAGE_LENGTH), length
                ).hex()
                for length in XOF_OUTPUT_LENGTHS
            },
        },
        "permutation": {
            "zero_state": permutation_vector([0] * 16),
            "counter_state": permutation_vector(list(range(16))),
        },
        "encoder": {
            "input": [str(value) for value in ENCODER_VECTOR_INPUT],
            "blocks": {
                str(round_index): {
                    str(block_index): encoder_block_vector(round_index, block_index)
                    for block_index in ENCODER_VECTOR_BLOCKS
                }
                for round_index in ENCODER_VECTOR_ROUNDS
            },
            "factor_stream": encoder_factor_vector(),
        },
    }

    vector_directory = REPOSITORY_ROOT / "test" / "vectors"
    kat_path = vector_directory / "ctc_sigma_v02_kat.json"
    manifest_path = vector_directory / "ctc_sigma_v02_encoder_constants.json"
    checksum_path = vector_directory / "ctc_sigma_v02_encoder_constants.sha256"

    write_json(kat_path, vectors)
    manifest_bytes = write_json(manifest_path, build_encoder_constant_manifest())
    digest = hashlib.sha256(manifest_bytes).hexdigest()
    checksum_path.write_text(f"{digest}  {manifest_path.name}\n", encoding="ascii")

    print(f"wrote {kat_path}")
    print(f"wrote {manifest_path}")
    print(f"wrote {checksum_path}")


if __name__ == "__main__":
    main()
