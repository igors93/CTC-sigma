#!/usr/bin/env python3
"""Generate the frozen known-answer vectors for CTC-Sigma v0.1.

Message convention (documented in the output file): a message of length n
consists of the bytes 0, 1, ..., (n-1) mod 256. The fixed XOF message is the
41-byte instance of the same pattern.

Run from the repository root after building the shared library:

    python scripts/generate_kat.py
"""

from __future__ import annotations

import ctypes
import json
import os
import sys
from pathlib import Path

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPOSITORY_ROOT / "test" / "python"))
os.environ.setdefault(
    "CTC_SIGMA_LIBRARY", str(REPOSITORY_ROOT / "build" / "libctc_sigma.so")
)

from ctc_bindings import CTC_STATUS_OK, hash256, lib, u64_array, xof  # noqa: E402

HASH_MESSAGE_LENGTHS = [0, 1, 39, 40, 41, 80, 1024]
XOF_OUTPUT_LENGTHS = [1, 31, 32, 40, 41, 64, 1000]
FIXED_XOF_MESSAGE_LENGTH = 41


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


def main() -> None:
    vectors = {
        "version": "CTC-Sigma v0.1",
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
    }

    output_path = REPOSITORY_ROOT / "test" / "vectors" / "ctc_sigma_v01_kat.json"
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(vectors, indent=2) + "\n")
    print(f"wrote {output_path}")


if __name__ == "__main__":
    main()
