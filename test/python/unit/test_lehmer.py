from __future__ import annotations

import ctypes

from ctc_bindings import CTC_STATUS_OK, lib


def test_rank_unrank_all_simple_factors():
    for rank in range(40320):
        permutation = (ctypes.c_uint8 * 8)()
        recovered_rank = ctypes.c_uint32()
        assert lib.ctc_lehmer_unrank(rank, permutation) == CTC_STATUS_OK
        assert sorted(permutation) == list(range(8))
        assert lib.ctc_lehmer_rank(permutation, ctypes.byref(recovered_rank)) == CTC_STATUS_OK
        assert recovered_rank.value == rank
