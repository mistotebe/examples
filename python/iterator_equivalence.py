# -*- coding: utf-8 -*-
"""
Checks that two iterables' contents are equivalent up to the first Ellipsis in
either.

Useful in tests where we only want to check a prefix of a(n infinite) sequence.
"""

from itertools import zip_longest


# implementation:
def iterators_equivalent(left, right):
    """
    Compare two iterators, return True if and only if equivalent
    up to first Ellipsis (which means 'do not care')
    """

    # construct a unique object that won't have existed anywhere else
    end = object()

    for l, r in zip_longest(left, right, fillvalue=end):
        # Ellipsis reached
        if l is Ellipsis or r is Ellipsis:
            return True

        # exactly one iterator finished
        if (l is end) != (r is end):
            return False

        # items do not match
        if l != r:
            return False

    # both iterators exhausted at the same time
    return True


# tests:
def test_exact():
    "Iterables that compare equal should still compare equal"

    assert iterators_equivalent([], [])
    assert iterators_equivalent((), ())

    assert iterators_equivalent([1], [1])
    assert iterators_equivalent('', "")
    assert iterators_equivalent('abc', 'abc')

    assert iterators_equivalent([0, 2] + [4, 6], list(range(0, 7, 2)))

    assert not iterators_equivalent([], [1])
    assert not iterators_equivalent([1], [2])
    assert not iterators_equivalent('1', [1])

    object_a = object()
    object_b = object()
    assert iterators_equivalent([object_a], [object_a])
    assert iterators_equivalent([object_b], [object_b])
    assert not iterators_equivalent([object_a], [object_b])


def test_approximate():
    """
    Compares only contents of iterators and only up to what the == operator
    can distinguish.
    """

    assert iterators_equivalent([], ())
    assert iterators_equivalent([1], [1.0])
    assert iterators_equivalent('abc', ['a', 'b', 'c'])
    assert iterators_equivalent((0, 2) + (4, 6), range(0, 7, 2))


def test_ellipsis():
    "Basic checks for Ellipsis handling"

    assert iterators_equivalent([Ellipsis], [Ellipsis])
    assert iterators_equivalent([Ellipsis], [])
    assert iterators_equivalent([Ellipsis], [None])
    assert iterators_equivalent([Ellipsis], [object()])
    assert iterators_equivalent([object()], [Ellipsis])

    assert iterators_equivalent([1, 2, 3], [1, ...])
    assert iterators_equivalent([1, 2, 3], [1, 2, ...])
    assert iterators_equivalent([1, 2, 3], [1, 2, 3, ...])
    assert not iterators_equivalent([1, 2, 3], [1, 2, 3, 4, ...])

    assert iterators_equivalent([0, ...], range(4))
    assert iterators_equivalent(['a', ...], 'abc')


def test_ellipsis_more():
    "Test with infite sequences etc."
    import itertools

    assert iterators_equivalent([0, 1, ...], itertools.count())
