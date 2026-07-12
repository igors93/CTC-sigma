from __future__ import annotations

import ctypes

from ctc_bindings import (
    BraidDescriptor,
    BraidNormalForm,
    CTC_FIELD_MODULUS,
    CTC_STATUS_OK,
    CTC_STATUS_OUT_OF_RANGE,
    DynamicArithConfig,
    NORMALIZER_CALLBACK,
    lib,
    u64_array,
)

CTC_ROUND_BRAID_ACTIVE = 0
CTC_ROUND_ARITHMETIC_BRIDGE = 1

CTC_BRANCH_VARIANT_FULL = 0
CTC_BRANCH_VARIANT_EMPTY_NORMAL_FORM = 1
CTC_BRANCH_VARIANT_FIXED_DESCRIPTOR = 2
CTC_BRANCH_VARIANT_NO_DYNAMIC_CONTROL = 3
CTC_BRANCH_VARIANT_ARITHMETIC_ONLY = 4


def make_normal_form(factor: int = 1, count: int = 20) -> BraidNormalForm:
    normal_form = BraidNormalForm()
    normal_form.infimum = -2
    normal_form.factor_count = count
    for index in range(count):
        normal_form.factors[index] = factor
    return normal_form


def test_round_schedule_is_b_b_a_repeated_four_times():
    observed = []
    for round_index in range(12):
        round_type = ctypes.c_int()
        assert lib.ctc_round_schedule_get(
            round_index, ctypes.byref(round_type)
        ) == CTC_STATUS_OK
        observed.append(round_type.value)

    assert observed == [
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_BRAID_ACTIVE,
        CTC_ROUND_ARITHMETIC_BRIDGE,
    ] * 4


def test_descriptor_controls_valid_dynamic_configuration():
    normal_form = make_normal_form()
    descriptor = BraidDescriptor()
    config = DynamicArithConfig()

    assert lib.ctc_braid_descriptor_build(
        ctypes.byref(normal_form), 0, ctypes.byref(descriptor)
    ) == CTC_STATUS_OK
    assert descriptor.token_count == 25
    assert all(0 <= value < CTC_FIELD_MODULUS for value in descriptor.lanes)

    assert lib.ctc_dynamic_arith_config_build(
        ctypes.byref(descriptor), 0, ctypes.byref(config)
    ) == CTC_STATUS_OK
    assert sorted(config.lane_permutation) == list(range(8))
    assert list(config.degrees).count(23) == 2
    assert list(config.degrees).count(47) == 2
    assert all(value != 0 for row in config.sbox_b for value in row)


def test_different_normal_forms_select_different_dynamic_transforms():
    first_descriptor = BraidDescriptor()
    second_descriptor = BraidDescriptor()
    first_config = DynamicArithConfig()
    second_config = DynamicArithConfig()

    assert lib.ctc_braid_descriptor_build(
        ctypes.byref(make_normal_form(1)), 3, ctypes.byref(first_descriptor)
    ) == CTC_STATUS_OK
    assert lib.ctc_braid_descriptor_build(
        ctypes.byref(make_normal_form(6)), 3, ctypes.byref(second_descriptor)
    ) == CTC_STATUS_OK
    assert list(first_descriptor.lanes) != list(second_descriptor.lanes)

    assert lib.ctc_dynamic_arith_config_build(
        ctypes.byref(first_descriptor), 3, ctypes.byref(first_config)
    ) == CTC_STATUS_OK
    assert lib.ctc_dynamic_arith_config_build(
        ctypes.byref(second_descriptor), 3, ctypes.byref(second_config)
    ) == CTC_STATUS_OK
    assert bytes(first_config) != bytes(second_config)


def test_braid_round_variants_are_observably_distinct():
    source = u64_array([11, 22, 33, 44, 55, 66, 77, 88])

    @NORMALIZER_CALLBACK
    def forwarding_normalizer(word, word_length, output, _context):
        return lib.ctc_braid_normalize_left(word, word_length, output, None)

    outputs = []
    for variant in range(5):
        output = (ctypes.c_uint64 * 8)()
        assert lib.ctc_branch_apply_variant(
            source,
            0,
            variant,
            forwarding_normalizer,
            None,
            output,
        ) == CTC_STATUS_OK
        outputs.append(tuple(output))

    assert len(set(outputs)) == len(outputs)


def test_bridge_round_is_shared_by_control_variants():
    source = u64_array([101, 202, 303, 404, 505, 606, 707, 808])

    @NORMALIZER_CALLBACK
    def forwarding_normalizer(word, word_length, output, _context):
        return lib.ctc_braid_normalize_left(word, word_length, output, None)

    outputs = []
    for variant in range(5):
        output = (ctypes.c_uint64 * 8)()
        assert lib.ctc_branch_apply_variant(
            source,
            2,
            variant,
            forwarding_normalizer,
            None,
            output,
        ) == CTC_STATUS_OK
        outputs.append(tuple(output))

    assert len(set(outputs)) == 1


def test_noncanonical_public_state_is_rejected_without_mutation():
    state_values = [CTC_FIELD_MODULUS] + list(range(1, 16))
    state = u64_array(state_values)
    assert lib.ctc_permutation_apply(state) == CTC_STATUS_OUT_OF_RANGE
    assert list(state) == state_values

    branch_state_values = [CTC_FIELD_MODULUS] + list(range(1, 8))
    branch_state = u64_array(branch_state_values)
    assert lib.ctc_arith_apply(b"V03-VALIDATION", 0, branch_state, 2) == CTC_STATUS_OUT_OF_RANGE
    assert list(branch_state) == branch_state_values

    output_values = [0xA5A5A5A5A5A5A5A5] * 8
    output = u64_array(output_values)
    assert lib.ctc_encoder_generate_block(
        branch_state, 0, 0, output
    ) == CTC_STATUS_OUT_OF_RANGE
    assert list(output) == output_values
