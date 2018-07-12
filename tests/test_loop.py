"""Loop tests."""
from concurrent.futures import Future
import gc
import logging
import pytest
import threading

from libchirp import Loop


def test_loop_lifecycle(caplog, ref_count_offset):
    """test_loop_lifecycle."""
    caplog.set_level(logging.DEBUG)
    a = Loop()
    try:
        a.run()
        assert len(gc.get_referrers(a)) > 1 + ref_count_offset
        assert a.running
        a.stop()
        assert not a.running
        assert len(gc.get_referrers(a)) == 1 + ref_count_offset
        assert caplog.record_tuples == [
            ('libchirp', 10, 'libuv event-loop started'),
            ('libchirp', 10, 'libuv event-loop stopped'),
        ]
    except BaseException:
        a.stop()
        raise


def get_thread(fut: Future):
    """get_thread."""
    fut.set_result(threading.current_thread())


def test_call_soon():
    """test_call_soon."""
    loop = Loop()
    try:
        assert loop.running
        fut = Future()
        loop.call_soon(get_thread, fut)
        fut.result() == loop._thread
    finally:
        loop.stop()
        assert not loop.running


def test_call_soon_reverse():
    """test_call_soon_reverse."""
    loop = Loop(False)
    try:
        fut = Future()
        loop.call_soon(get_thread, fut)
        assert not loop.running
        loop.run()
        assert loop.running
        fut.result() == loop._thread
    finally:
        loop.stop()
        assert not loop.running


def test_start_stop_orders():
    """test_start_stop_orders."""
    loop = Loop(False)
    try:
        assert not loop.running
        loop.stop()
        assert not loop.running
        loop.run()
        assert loop.running
        loop.run()
        assert loop.running
        loop.stop()
        assert not loop.running
        with pytest.raises(RuntimeError):
            loop.run()
    finally:
        loop.stop()
        assert not loop.running
