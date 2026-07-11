from __future__ import annotations

import ctypes

from ctc_bindings import (
    BraidNormalForm,
    CTC_STATUS_OK,
    NORMALIZER_CALLBACK,
    lib,
    u64_array,
)


@NORMALIZER_CALLBACK
def invalid_identity_normalizer(_word, _word_length, normal_form, _context):
    target: BraidNormalForm = normal_form.contents
    target.infimum = 0
    target.factor_count = 1
    target.factors[0] = 0
    return CTC_STATUS_OK


@NORMALIZER_CALLBACK
def invalid_delta_normalizer(_word, _word_length, normal_form, _context):
    target: BraidNormalForm = normal_form.contents
    target.infimum = 0
    target.factor_count = 1
    target.factors[0] = 40319
    return CTC_STATUS_OK


@NORMALIZER_CALLBACK
def invalid_non_left_weighted_normalizer(_word, _word_length, normal_form, _context):
    target: BraidNormalForm = normal_form.contents
    target.infimum = 0
    target.factor_count = 2
    target.factors[0] = 1
    target.factors[1] = 2
    return CTC_STATUS_OK


def test_branch_rejects_malformed_injected_normal_forms_before_fold():
    right_half = u64_array([0] * 8)
    output = (ctypes.c_uint64 * 8)()

    for normalizer in (
        invalid_identity_normalizer,
        invalid_delta_normalizer,
        invalid_non_left_weighted_normalizer,
    ):
        assert lib.ctc_branch_apply_with_normalizer(
            right_half, 0, normalizer, None, output
        ) != CTC_STATUS_OK
