"""Independent Python reference for the left Garside normal form in B_8.

This module deliberately avoids the meet-based algorithm used by the C core.
Left-weighting is performed by moving one generator at a time from the head
of the right factor to the tail of the left factor, using the classical
starting-set / finishing-set characterization. Agreement between this module
and ``src/braid.c`` is therefore a meaningful cross-check.

Convention (identical to the one documented in ``src/braid.c``):
  - a simple element is a tuple ``perm`` with ``perm[start] = end``;
  - products are read left to right: ``(x * y)[s] = y[x[s]]``;
  - spec generator sigma_(j+1) is the transposition of positions j, j+1;
  - Delta is ``(7, 6, ..., 0)`` and has Lehmer rank 40319.
"""

from __future__ import annotations

STRANDS = 8
IDENTITY = tuple(range(STRANDS))
DELTA = tuple(range(STRANDS - 1, -1, -1))

_FACTORIALS = [1, 1, 2, 6, 24, 120, 720, 5040, 40320]


def lehmer_unrank(rank: int) -> tuple[int, ...]:
    if not 0 <= rank < _FACTORIALS[STRANDS]:
        raise ValueError(f"rank out of range: {rank}")
    available = list(range(STRANDS))
    result = []
    for position in range(STRANDS):
        factorial = _FACTORIALS[STRANDS - 1 - position]
        selected, rank = divmod(rank, factorial)
        result.append(available.pop(selected))
    return tuple(result)


def lehmer_rank(perm: tuple[int, ...]) -> int:
    rank = 0
    for position in range(STRANDS):
        smaller = sum(
            1
            for following in range(position + 1, STRANDS)
            if perm[following] < perm[position]
        )
        rank += smaller * _FACTORIALS[STRANDS - 1 - position]
    return rank


def product(x: tuple[int, ...], y: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(y[x[s]] for s in range(STRANDS))


def inverse(x: tuple[int, ...]) -> tuple[int, ...]:
    result = [0] * STRANDS
    for start, end in enumerate(x):
        result[end] = start
    return tuple(result)


def complement(x: tuple[int, ...]) -> tuple[int, ...]:
    """Right complement d(x) = inverse(x) * Delta, so x * d(x) = Delta."""
    return tuple(STRANDS - 1 - value for value in inverse(x))


def tau(x: tuple[int, ...]) -> tuple[int, ...]:
    """Flip automorphism tau(x) = Delta^-1 * x * Delta."""
    return tuple(STRANDS - 1 - x[STRANDS - 1 - s] for s in range(STRANDS))


def starting_set(x: tuple[int, ...]) -> set[int]:
    return {j for j in range(STRANDS - 1) if x[j] > x[j + 1]}


def finishing_set(x: tuple[int, ...]) -> set[int]:
    return starting_set(inverse(x))


def _append_sigma(x: tuple[int, ...], j: int) -> tuple[int, ...]:
    """x * sigma_j: swap the output values j and j+1."""
    swap = {j: j + 1, j + 1: j}
    return tuple(swap.get(value, value) for value in x)


def _strip_sigma(x: tuple[int, ...], j: int) -> tuple[int, ...]:
    """sigma_j^-1 * x, defined when j is in the starting set of x."""
    values = list(x)
    values[j], values[j + 1] = values[j + 1], values[j]
    return tuple(values)


def make_left_weighted(
    left: tuple[int, ...], right: tuple[int, ...]
) -> tuple[tuple[int, ...], tuple[int, ...], bool]:
    changed = False
    while True:
        movable = starting_set(right) - finishing_set(left)
        if not movable:
            return left, right, changed
        generator = min(movable)
        left = _append_sigma(left, generator)
        right = _strip_sigma(right, generator)
        changed = True


def normalize(word: list[tuple[int, int]]) -> tuple[int, list[int]]:
    """Left Garside normal form of a word of signed simple factors.

    ``word`` is a list of ``(lehmer_rank, sign)`` pairs with sign +1 or -1.
    Returns ``(infimum, [proper factor ranks])``.
    """
    simples: list[tuple[int, ...]] = []
    infimum = 0
    for rank, sign in word:
        if sign not in (1, -1):
            raise ValueError(f"invalid sign: {sign}")
        simple = lehmer_unrank(rank)
        if simple == IDENTITY:
            continue
        if sign > 0:
            simples.append(simple)
        else:
            simples = [tau(entry) for entry in simples]
            infimum -= 1
            simples.append(tau(complement(simple)))

    changed = True
    while changed:
        changed = False
        for index in range(len(simples) - 1):
            left, right, moved = make_left_weighted(
                simples[index], simples[index + 1]
            )
            simples[index], simples[index + 1] = left, right
            changed = changed or moved

    while simples and simples[0] == DELTA:
        simples.pop(0)
        infimum += 1
    while simples and simples[-1] == IDENTITY:
        simples.pop()
    return infimum, [lehmer_rank(entry) for entry in simples]


def word_permutation(word: list[tuple[int, int]]) -> tuple[int, ...]:
    """Projection of a signed word to S_8, for sanity checks."""
    result = IDENTITY
    for rank, sign in word:
        simple = lehmer_unrank(rank)
        result = product(result, simple if sign > 0 else inverse(simple))
    return result


def normal_form_permutation(infimum: int, ranks: list[int]) -> tuple[int, ...]:
    result = IDENTITY
    delta_power = DELTA if infimum >= 0 else inverse(DELTA)
    for _ in range(abs(infimum)):
        result = product(result, delta_power)
    for rank in ranks:
        result = product(result, lehmer_unrank(rank))
    return result
