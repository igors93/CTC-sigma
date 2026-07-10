from __future__ import annotations

import ctypes

from ctc_bindings import CTC_STATUS_OK, lib


def encode(message: bytes, domain: int = 1) -> bytes:
    input_buffer = (ctypes.c_uint8 * len(message)).from_buffer_copy(message) if message else None
    output_pointer = ctypes.POINTER(ctypes.c_uint8)()
    output_length = ctypes.c_size_t()
    status = lib.ctc_sponge_encode_message(
        input_buffer,
        len(message),
        domain,
        ctypes.byref(output_pointer),
        ctypes.byref(output_length),
    )
    assert status == CTC_STATUS_OK
    try:
        return ctypes.string_at(output_pointer, output_length.value)
    finally:
        lib.ctc_sponge_free_encoded_message(output_pointer)


def test_message_encoding_and_padding_for_boundary_lengths():
    for length in [0, 1, 39, 40, 41, 80, 1024]:
        message = bytes((index * 17) % 256 for index in range(length))
        encoded = encode(message)
        assert len(encoded) % 40 == 0
        assert encoded[:length] == message
        assert encoded[length : length + 8] == length.to_bytes(8, "little")
        assert encoded[length + 8] == 0x01
        assert encoded[length + 9] == 0x01
        assert encoded[-1] == 0x80
        assert set(encoded[length + 10 : -1]) <= {0}
