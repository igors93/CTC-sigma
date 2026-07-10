from __future__ import annotations

from ctc_bindings import CTC_STATUS_NOT_IMPLEMENTED, lib, u64_array


def test_default_permutation_reports_missing_exact_garside_normalizer():
    original = list(range(16))
    state = u64_array(original)
    status = lib.ctc_permutation_apply(state)
    assert status == CTC_STATUS_NOT_IMPLEMENTED
    assert list(state) == original
