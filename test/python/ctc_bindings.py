from __future__ import annotations

import ctypes
import os
from pathlib import Path

CTC_STATUS_OK = 0
CTC_STATUS_INVALID_ARGUMENT = 1
CTC_STATUS_OUT_OF_RANGE = 2
CTC_STATUS_NOT_IMPLEMENTED = 6

CTC_ENCODER_CONSTANT_RC = 0
CTC_ENCODER_CONSTANT_SBOX_A = 1
CTC_ENCODER_CONSTANT_SBOX_B = 2
CTC_ENCODER_CONSTANT_SBOX_C = 3
CTC_FIELD_MODULUS = 2**61 - 1
CTC_FACTORS_PER_BRANCH = 32
CTC_MAX_NORMAL_FACTORS = 512
CTC_MAX_FOLD_TOKENS = CTC_MAX_NORMAL_FACTORS + 8


class SignedFactor(ctypes.Structure):
    _fields_ = [
        ("simple_index", ctypes.c_uint16),
        ("sign", ctypes.c_int8),
    ]


class BraidNormalForm(ctypes.Structure):
    _fields_ = [
        ("infimum", ctypes.c_int64),
        ("factor_count", ctypes.c_size_t),
        ("factors", ctypes.c_uint16 * CTC_MAX_NORMAL_FACTORS),
    ]


class FoldTokens(ctypes.Structure):
    _fields_ = [
        ("prefix_factor_count", ctypes.c_size_t),
        ("token_count", ctypes.c_size_t),
        ("values", ctypes.c_uint64 * CTC_MAX_FOLD_TOKENS),
    ]


class Sponge(ctypes.Structure):
    _fields_ = [
        ("state", ctypes.c_uint64 * 16),
        ("domain", ctypes.c_int),
    ]


NORMALIZER_CALLBACK = ctypes.CFUNCTYPE(
    ctypes.c_int,
    ctypes.POINTER(SignedFactor),
    ctypes.c_size_t,
    ctypes.POINTER(BraidNormalForm),
    ctypes.c_void_p,
)


def _load_library() -> ctypes.CDLL:
    raw_path = os.environ.get("CTC_SIGMA_LIBRARY")
    if not raw_path:
        raise RuntimeError("CTC_SIGMA_LIBRARY is not configured")
    path = Path(raw_path)
    if not path.exists():
        raise RuntimeError(
            f"Shared library not found at {path}. Build with: cmake -S . -B build && cmake --build build"
        )
    return ctypes.CDLL(str(path))


lib = _load_library()

lib.ctc_sigma_version.argtypes = []
lib.ctc_sigma_version.restype = ctypes.c_char_p

lib.ctc_field_add.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
lib.ctc_field_add.restype = ctypes.c_uint64
lib.ctc_field_sub.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
lib.ctc_field_sub.restype = ctypes.c_uint64
lib.ctc_field_mul.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
lib.ctc_field_mul.restype = ctypes.c_uint64
lib.ctc_field_pow.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
lib.ctc_field_pow.restype = ctypes.c_uint64
lib.ctc_field_inv.argtypes = [ctypes.c_uint64]
lib.ctc_field_inv.restype = ctypes.c_uint64
lib.ctc_field_dickson.argtypes = [ctypes.c_uint64, ctypes.c_uint64]
lib.ctc_field_dickson.restype = ctypes.c_uint64

lib.ctc_constant_derive.argtypes = [
    ctypes.c_char_p,
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_uint64),
]
lib.ctc_constant_derive.restype = ctypes.c_int

lib.ctc_encoder_constant_derive.argtypes = [
    ctypes.c_int,
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_uint64),
]
lib.ctc_encoder_constant_derive.restype = ctypes.c_int

lib.ctc_arith_apply.argtypes = [
    ctypes.c_char_p,
    ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.c_uint32,
]
lib.ctc_arith_apply.restype = ctypes.c_int
lib.ctc_arith_inverse.argtypes = lib.ctc_arith_apply.argtypes
lib.ctc_arith_inverse.restype = ctypes.c_int

lib.ctc_arith_apply_encoder.argtypes = [
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.c_uint32,
]
lib.ctc_arith_apply_encoder.restype = ctypes.c_int
lib.ctc_arith_inverse_encoder.argtypes = lib.ctc_arith_apply_encoder.argtypes
lib.ctc_arith_inverse_encoder.restype = ctypes.c_int

lib.ctc_mds_matrix.argtypes = [ctypes.POINTER((ctypes.c_uint64 * 8) * 8)]
lib.ctc_mds_matrix.restype = ctypes.c_int
lib.ctc_mds_inverse.argtypes = [ctypes.POINTER((ctypes.c_uint64 * 8) * 8)]
lib.ctc_mds_inverse.restype = ctypes.c_int

lib.ctc_lehmer_rank.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.POINTER(ctypes.c_uint32),
]
lib.ctc_lehmer_rank.restype = ctypes.c_int
lib.ctc_lehmer_unrank.argtypes = [ctypes.c_uint32, ctypes.POINTER(ctypes.c_uint8)]
lib.ctc_lehmer_unrank.restype = ctypes.c_int

lib.ctc_braid_normalize_left.argtypes = [
    ctypes.POINTER(SignedFactor),
    ctypes.c_size_t,
    ctypes.POINTER(BraidNormalForm),
    ctypes.c_void_p,
]
lib.ctc_braid_normalize_left.restype = ctypes.c_int

lib.ctc_braid_validate_normal_form.argtypes = [ctypes.POINTER(BraidNormalForm)]
lib.ctc_braid_validate_normal_form.restype = ctypes.c_int

lib.ctc_encoder_generate_block.argtypes = [
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.c_uint32,
    ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_uint64),
]
lib.ctc_encoder_generate_block.restype = ctypes.c_int

lib.ctc_encoder_generate_factors.argtypes = [
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.c_uint32,
    ctypes.POINTER(SignedFactor),
    ctypes.POINTER(ctypes.c_size_t),
]
lib.ctc_encoder_generate_factors.restype = ctypes.c_int

lib.ctc_branch_apply_with_normalizer.argtypes = [
    ctypes.POINTER(ctypes.c_uint64),
    ctypes.c_uint32,
    NORMALIZER_CALLBACK,
    ctypes.c_void_p,
    ctypes.POINTER(ctypes.c_uint64),
]
lib.ctc_branch_apply_with_normalizer.restype = ctypes.c_int

lib.ctc_fold_tokenize_normal_form.argtypes = [
    ctypes.POINTER(BraidNormalForm),
    ctypes.c_uint32,
    ctypes.POINTER(FoldTokens),
]
lib.ctc_fold_tokenize_normal_form.restype = ctypes.c_int
lib.ctc_fold_normal_form.argtypes = [
    ctypes.POINTER(BraidNormalForm),
    ctypes.c_uint32,
    ctypes.POINTER(ctypes.c_uint64),
]
lib.ctc_fold_normal_form.restype = ctypes.c_int

lib.ctc_sponge_initialize.argtypes = [ctypes.POINTER(Sponge), ctypes.c_int]
lib.ctc_sponge_initialize.restype = ctypes.c_int
lib.ctc_sponge_encode_message.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.c_int,
    ctypes.POINTER(ctypes.POINTER(ctypes.c_uint8)),
    ctypes.POINTER(ctypes.c_size_t),
]
lib.ctc_sponge_encode_message.restype = ctypes.c_int
lib.ctc_sponge_free_encoded_message.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
lib.ctc_sponge_free_encoded_message.restype = None

lib.ctc_permutation_apply.argtypes = [ctypes.POINTER(ctypes.c_uint64)]
lib.ctc_permutation_apply.restype = ctypes.c_int
lib.ctc_permutation_inverse.argtypes = [ctypes.POINTER(ctypes.c_uint64)]
lib.ctc_permutation_inverse.restype = ctypes.c_int
lib.ctc_permutation_apply_with_normalizer.argtypes = [
    ctypes.POINTER(ctypes.c_uint64),
    NORMALIZER_CALLBACK,
    ctypes.c_void_p,
]
lib.ctc_permutation_apply_with_normalizer.restype = ctypes.c_int
lib.ctc_permutation_inverse_with_normalizer.argtypes = [
    ctypes.POINTER(ctypes.c_uint64),
    NORMALIZER_CALLBACK,
    ctypes.c_void_p,
]
lib.ctc_permutation_inverse_with_normalizer.restype = ctypes.c_int

lib.ctc_hash256_with_normalizer.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    NORMALIZER_CALLBACK,
    ctypes.c_void_p,
    ctypes.POINTER(ctypes.c_uint8),
]
lib.ctc_hash256_with_normalizer.restype = ctypes.c_int
lib.ctc_xof_with_normalizer.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    NORMALIZER_CALLBACK,
    ctypes.c_void_p,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
lib.ctc_xof_with_normalizer.restype = ctypes.c_int

lib.ctc_hash256.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_uint8),
]
lib.ctc_hash256.restype = ctypes.c_int
lib.ctc_xof.argtypes = [
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
    ctypes.POINTER(ctypes.c_uint8),
    ctypes.c_size_t,
]
lib.ctc_xof.restype = ctypes.c_int


def hash256(message: bytes) -> bytes:
    source = (
        (ctypes.c_uint8 * len(message)).from_buffer_copy(message)
        if message
        else None
    )
    digest = (ctypes.c_uint8 * 32)()
    status = lib.ctc_hash256(source, len(message), digest)
    if status != CTC_STATUS_OK:
        raise RuntimeError(f"ctc_hash256 failed with status {status}")
    return bytes(digest)


def xof(message: bytes, output_length: int) -> bytes:
    source = (
        (ctypes.c_uint8 * len(message)).from_buffer_copy(message)
        if message
        else None
    )
    output = (ctypes.c_uint8 * output_length)()
    status = lib.ctc_xof(source, len(message), output, output_length)
    if status != CTC_STATUS_OK:
        raise RuntimeError(f"ctc_xof failed with status {status}")
    return bytes(output)


def u64_array(values: list[int] | tuple[int, ...], length: int | None = None):
    expected = len(values) if length is None else length
    if len(values) != expected:
        raise ValueError(f"expected {expected} values, got {len(values)}")
    return (ctypes.c_uint64 * expected)(*values)
