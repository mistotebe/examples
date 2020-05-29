# -*- coding: utf-8 -*-
"""
Mimics parts of Qt signals implementation.
"""

import asyncio

from collections.abc import Awaitable
from typing import Callable, Optional

class Signal:
    class _SignalInstance:
        def __init__(self, signal):
            self.connections = set()
            self._signal = signal

        def __repr__(self):
            return "Signal(%r)" % (self._signal.name)

        def connect(self, slot: Callable):
            self.connections.add(slot)

        def disconnect(self, slot: Optional[Callable]=None):
            if slot is None:
                return self.connections.clear()

            return self.connections.remove(slot)

        def __call__(self, *args, **kwargs):
            return self.emit(*args, **kwargs)

        def emit(self, *args, **kwargs):
            for slot in self.connections:
                slot(*args, **kwargs)

        async def aemit(self, *args, **kwargs):
            awaitables = []

            # Schedule them all at once
            for slot in self.connections:
                result = slot(*args, **kwargs)
                if isinstance(result, Awaitable):
                    awaitables.append(result)

            if awaitables:
                await asyncio.wait(awaitables)

        async def __await__(self):
            loop = asyncio.get_running_loop()
            future = loop.create_future()

            def callback(*args, **kwargs):
                future.set_result((args, kwargs))

            self.connect(callback)
            try:
                return await future
            finally:
                self.disconnect(callback)

        async def __aiter__(self):
            queue = asyncio.Queue()
            def callback(*args, **kwargs):
                queue.put_nowait((args, kwargs))

            self.connect(callback)
            try:
                while True:
                    yield await queue.get()
            finally:
                self.disconnect(callback)

    def __set_name__(self, owner, name):
        self.name = name

    def __get__(self, instance, owner=None):
        instance.__dict__[self.name] = Signal._SignalInstance(self)
        return instance.__dict__[self.name]


def test_basics():
    class Ping:
        signal = Signal()

        def __init__(self, name):
            self.name = name

        def handle(self, history, value):
            history[(self, value)] = history.get((self, value), 0) + 1

    def handler(history, value):
        history[(None, value)] = history.get((None, value), 0) + 1

    a = Ping('a')
    b = Ping('b')

    a.signal.connect(b.signal)
    a.signal.connect(a.handle)
    a.signal.connect(handler)
    b.signal.connect(b.handle)
    b.signal.connect(handler)

    history = {}
    a.signal(history, 1) # a.signal (a.handle) -> b.signal (b.handle, handler)
    assert history == {(None, 1): 2, (a, 1): 1, (b, 1): 1}, "Propagation failed"

    history = {}
    b.signal(history, 'value') # b.signal (b.handle, handler)
    assert history == {(None, 'value'): 1, (b, 'value'): 1}, "Propagation failed"

    a.signal.disconnect(b.signal)
    a.signal.connect(b.handle)
    a.signal.disconnect(handler)

    history = {}
    a.signal(history, 2) # a.signal (a.handle, b.handle)
    assert history == {(a, 2): 1, (b, 2): 1}, "Propagation failed"
