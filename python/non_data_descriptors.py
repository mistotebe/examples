# -*- coding: utf-8 -*-
"""
If an attribute defines __get__ it is a descriptor. If it also defines __set__
or __delete__, it is a so called "data descriptor", otherwise it's a "non-data
descriptor".

Data descriptors intercept all accesses to the attribute, where non-data
descriptors are ignored if instance has a corresponding item in its __dict__.

We can use it to populate the value on first access like below.
"""

import functools


# implementation:
class Lazy:
    """
    Descriptor where a value needs to be populated, but unlike property, this
    only needs to happen on first access.
    """

    def __init__(self, resolver, names=None):
        self.resolver = resolver
        self.names = names
        self.name = None

    def __set_name__(self, owner, name):
        if self.names:
            setattr(owner, name, self.resolver)
            for name in self.names:
                prop = self.__class__(self.resolver)
                setattr(owner, name, prop)
                prop.__set_name__(owner, name)
            return
        self.name = name

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self
        self.resolver(obj)
        return getattr(obj, self.name)


def lazy(*args):
    def decorator_lazy(func):
        return Lazy(func, names)

    if len(args) == 1 and callable(args[0]):
        return Lazy(args[0])

    names = args
    return decorator_lazy


# tests:
def test_direct():
    class Direct:
        def __init__(self):
            self.history = []

        def attr_resolver(self):
            value = object()
            self.history.append(value)
            self.attr = value

        attr = Lazy(attr_resolver)

    one = Direct()
    two = Direct()

    assert one.history == [], "attr already resolved"
    assert one.attr is one.attr, "attr_resolver called more than once"
    assert one.history == [one.attr], "attr_resolver called more than once"

    assert two.history == [], "attr already resolved"
    assert two.attr is two.attr, "attr_resolver called more than once"
    assert two.history == [two.attr], "attr_resolver called more than once"

    assert one.attr is not two.attr, "data leaked between instances"

    one.attr = 'value'
    assert one.attr == 'value', "assignment failed"

    three = Direct()
    three.attr = 'value'
    assert three.attr == 'value', "assignment failed"
    assert three.history == [], "attempted to resolve again"


def test_decorator():
    class Decorated:
        def __init__(self):
            self.history = []

        @lazy
        def attr(self):
            value = object()
            self.history.append(value)
            self.attr = value

    one = Decorated()
    two = Decorated()

    assert one.history == [], "attr already resolved"
    assert one.attr is one.attr, "attr called more than once"
    assert one.history == [one.attr], "attr called more than once"

    assert two.history == [], "attr already resolved"
    assert two.attr is two.attr, "attr called more than once"
    assert two.history == [two.attr], "attr called more than once"

    assert one.attr is not two.attr, "data leaked between instances"

    one.attr = 'value'
    assert one.attr == 'value', "assignment failed"

    three = Decorated()
    three.attr = 'value'
    assert three.attr == 'value', "assignment failed"
    assert three.history == [], "attempted to resolve again"


def test_decorator_multi():
    class DecoratedMulti:
        def __init__(self):
            self.history = []

        @lazy('a', 'b', 'c')
        def resolver(self):
            a, b = object(), object()
            c = b
            self.history.append((a, b, c))
            self.a = a
            self.b = b
            self.c = c

    multi = DecoratedMulti()

    assert multi.history == [], "attributes already resolved"
    assert multi.b is multi.c, "not resolved correctly"
    assert multi.a is multi.a, "resolver called more than once"
    assert multi.a != multi.b, "not resolved correctly"
    assert multi.history == [(multi.a, multi.b, multi.c)], "resolver called more than once"
