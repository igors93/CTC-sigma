from __future__ import annotations

import ctypes
import hashlib

from ctc_bindings import (
    CTC_ENCODER_CONSTANT_RC,
    CTC_ENCODER_CONSTANT_SBOX_A,
    CTC_ENCODER_CONSTANT_SBOX_B,
    CTC_ENCODER_CONSTANT_SBOX_C,
    CTC_FIELD_MODULUS,
    CTC_STATUS_INVALID_ARGUMENT,
    CTC_STATUS_OK,
    CTC_STATUS_OUT_OF_RANGE,
    lib,
)

ENCODER_COMPONENT_PURPOSES = {
    CTC_ENCODER_CONSTANT_RC: 1,
    CTC_ENCODER_CONSTANT_SBOX_A: 2,
    CTC_ENCODER_CONSTANT_SBOX_B: 3,
    CTC_ENCODER_CONSTANT_SBOX_C: 4,
}


def derive_python(label: bytes, first_index: int, second_index: int) -> int:
    seed = (
        b"CTC-SIGMA-v0.3|CONST|"
        + bytes([len(label)])
        + label
        + (0).to_bytes(4, "little")
        + first_index.to_bytes(4, "little")
        + second_index.to_bytes(4, "little")
        + (0).to_bytes(4, "little")
        + (0).to_bytes(4, "little")
    )
    return int.from_bytes(hashlib.shake_256(seed).digest(16), "little") % CTC_FIELD_MODULUS


def derive_encoder_python(
    component: int,
    round_index: int,
    block_index: int,
    subround_index: int,
    lane_index: int,
) -> int:
    component_name = b"A_ENC"
    seed = (
        b"CTC-SIGMA-v0.3|CONST|"
        + bytes([len(component_name)])
        + component_name
        + ENCODER_COMPONENT_PURPOSES[component].to_bytes(4, "little")
        + round_index.to_bytes(4, "little")
        + subround_index.to_bytes(4, "little")
        + lane_index.to_bytes(4, "little")
        + block_index.to_bytes(4, "little")
    )
    value = int.from_bytes(hashlib.shake_256(seed).digest(16), "little") % CTC_FIELD_MODULUS
    return 1 if component == CTC_ENCODER_CONSTANT_SBOX_B and value == 0 else value


def derive_encoder_c(
    component: int,
    round_index: int,
    block_index: int,
    subround_index: int,
    lane_index: int,
) -> int:
    result = ctypes.c_uint64()
    status = lib.ctc_encoder_constant_derive(
        component,
        round_index,
        block_index,
        subround_index,
        lane_index,
        ctypes.byref(result),
    )
    assert status == CTC_STATUS_OK
    return result.value


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


def test_encoder_constant_derivation_matches_normative_python_encoding():
    for component in ENCODER_COMPONENT_PURPOSES:
        for round_index, block_index, subround_index, lane_index in [
            (0, 0, 0, 0),
            (3, 1, 2, 7),
            (7, 4, 3, 5),
            (11, 1_048_575, 1, 2),
        ]:
            assert derive_encoder_c(
                component,
                round_index,
                block_index,
                subround_index,
                lane_index,
            ) == derive_encoder_python(
                component,
                round_index,
                block_index,
                subround_index,
                lane_index,
            )


def test_encoder_constants_are_domain_separated_by_block_and_component():
    values_by_component = {
        component: derive_encoder_c(component, 3, 0, 2, 5)
        for component in ENCODER_COMPONENT_PURPOSES
    }
    assert len(set(values_by_component.values())) == len(values_by_component)

    for component in ENCODER_COMPONENT_PURPOSES:
        block_values = {
            derive_encoder_c(component, 3, block_index, 2, 5)
            for block_index in range(5)
        }
        assert len(block_values) == 5


def test_encoder_constant_derivation_rejects_invalid_domain_fields():
    output = ctypes.c_uint64()
    assert lib.ctc_encoder_constant_derive(
        99, 0, 0, 0, 0, ctypes.byref(output)
    ) == CTC_STATUS_INVALID_ARGUMENT
    assert lib.ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_RC, 12, 0, 0, 0, ctypes.byref(output)
    ) == CTC_STATUS_OUT_OF_RANGE
    assert lib.ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_RC, 0, 1_048_576, 0, 0, ctypes.byref(output)
    ) == CTC_STATUS_OUT_OF_RANGE
    assert lib.ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_RC, 0, 0, 4, 0, ctypes.byref(output)
    ) == CTC_STATUS_OUT_OF_RANGE
    assert lib.ctc_encoder_constant_derive(
        CTC_ENCODER_CONSTANT_RC, 0, 0, 0, 8, ctypes.byref(output)
    ) == CTC_STATUS_OUT_OF_RANGE


def test_reported_version_is_v03_development_line():
    assert lib.ctc_sigma_version().decode("ascii") == "0.3.0-dev"
