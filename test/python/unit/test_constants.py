from __future__ import annotations

import ctypes
import hashlib

from ctc_bindings import CTC_FIELD_MODULUS, CTC_STATUS_OK, lib


def derive_python(label: bytes, first_index: int, second_index: int) -> int:
    seed = (
        b"CTC-SIGMA-v0.1|"
        + label
        + first_index.to_bytes(4, "little")
        + second_index.to_bytes(4, "little")
    )
    return int.from_bytes(hashlib.shake_256(seed).digest(16), "little") % CTC_FIELD_MODULUS


def test_public_constant_derivation_matches_python_shake256():
    for label, first_index, second_index in [
        (b"IV-HASH", 0, 0),
        (b"IV-XOF", 0, 7),
        (b"A_PRE|SBOX-A", 3, 1024),
        (b"FOLD-IV", 11, 5),
    ]:
        result = ctypes.c_uint64()
        status = lib.ctc_constant_derive(label, first_index, second_index, ctypes.byref(result))
        assert status == CTC_STATUS_OK
        assert result.value == derive_python(label, first_index, second_index)
