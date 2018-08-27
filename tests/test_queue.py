"""Queue tests."""

import queue
import time
import gc

from libchirp.queue import Chirp, Config, Message


def test_initialize(loop, config):
    """test_initialize."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = Chirp(loop, config)
    a.stop()


def test_request(config, sender, message, ref_count_offset):
    """test_request."""
    try:
        config = Config()
        config.DH_PARAMS_PEM = "./tests/dh.pem"
        config.CERT_CHAIN_PEM = "./tests/cert.pem"
        config.SYNCHRONOUS = False
        config.AUTO_RELEASE = False
        a = Chirp(sender.loop, config)
        message.data = b'hello'
        message.address = "127.0.0.1"
        message.port = config.PORT
        fut = sender.request(message)
        msg = a.get()
        assert msg.data == b'hello'
        a.send(msg).result()
        b = msg.release_slot()
        b.result()
        assert msg._msg_t is None
        msg.release()
        assert msg._msg_t is not None
        msg2 = fut.result()
        assert msg2._msg_t is None
        msg2.release()
        assert msg2._msg_t is not None
        p = fut.send_result()
        assert p is message
    finally:
        a.stop()
        msg = None
        msg2 = None
        assert len(gc.get_referrers(a)) == 1 + ref_count_offset


def test_recv_msg(config, sender, message, ref_count_offset):
    """test_recv_msg."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = False
    a = Chirp(sender.loop, config)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message)
    msg = a.get()
    assert msg.data == b'hello'
    msg.release_slot().result()
    assert msg._msg_t is None
    msg.release()
    assert msg._msg_t is not None
    a.stop()
    msg = None
    assert len(gc.get_referrers(a)) == 1 + ref_count_offset


def test_disable_queue(config, sender, message):
    """test_disable_queue."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = Chirp(sender.loop, config)
    a.disable_queue = True
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message).result()
    assert a.empty()
    a.stop()


def test_recv_msg_wait(config, sender, message):
    """test_recv_msg_wait."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = Chirp(sender.loop, config)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    fut = sender.send(message)
    a.get()
    fut.result()
    a.stop()


def test_recv_msg_no_wait(config, sender, message):
    """test_recv_msg_no_wait."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = Chirp(sender.loop, config)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    fut = sender.send(message)
    try:
        a.get_nowait()
    except queue.Empty:
        a.get()
    fut.result()
    a.stop()


def test_recv_msg_perf(capsys, config, sender):
    """test_recv_msg_perf."""
    try:
        config = Config()
        config.DH_PARAMS_PEM = "./tests/dh.pem"
        config.CERT_CHAIN_PEM = "./tests/cert.pem"
        config.AUTO_RELEASE = False
        a = Chirp(sender.loop, config)
        messages = []
        for _ in range(100):
            msg = Message()
            msg.data = b'hello'
            msg.address = "127.0.0.1"
            msg.port = config.PORT
            messages.append(msg)
        start = time.time()
        for _ in range(100):
            futs = []
            for msg in messages:
                futs.append(sender.send(msg))
            for _ in range(100):
                a.get().release()
            for fut in futs:
                fut.result()
        end = time.time()
        with capsys.disabled():
            print("\n%d msg/s" % (10000 / (end - start)))
    finally:
        a.stop()


def test_recv_msg_perf_fast(capsys, config, fast_sender):
    """test_recv_msg_perf_fast."""
    try:
        config = Config()
        config.DISABLE_ENCRYPTION = True
        config.SYNCHRONOUS = False
        a = Chirp(fast_sender.loop, config)
        messages = []
        for _ in range(100):
            msg = Message()
            msg.data = b'hello'
            msg.address = "127.0.0.1"
            msg.port = config.PORT
            messages.append(msg)
        start = time.time()
        for _ in range(100):
            futs = []
            for msg in messages:
                futs.append(fast_sender.send(msg))
            for _ in range(100):
                a.get()
            for fut in futs:
                fut.result()
        end = time.time()
        with capsys.disabled():
            print("\n%d msg/s" % (10000 / (end - start)))
    finally:
        a.stop()
