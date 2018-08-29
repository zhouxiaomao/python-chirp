"""Queue tests."""

import asyncio
import gc
import pytest
import time

from libchirp.asyncio import Chirp, Config


def test_request_async(config, queue, message, ref_count_offset):
    """test_request_async."""
    try:
        aio_loop = asyncio.get_event_loop()
        config = Config()
        config.DH_PARAMS_PEM = "./tests/dh.pem"
        config.CERT_CHAIN_PEM = "./tests/cert.pem"
        config.SYNCHRONOUS = False
        config.AUTO_RELEASE = False
        a = Chirp(queue.loop, config, aio_loop)
        message.data = b'hello'
        message.address = "127.0.0.1"
        message.port = 2992
        fut = a.request(message)
        p = aio_loop.run_until_complete(fut.send_result())
        assert p is message
        aio_loop.run_until_complete(fut.send_result())
        msg = queue.get()
        send_fut = queue.send(msg)
        assert msg.data == b'hello'
        msg2 = aio_loop.run_until_complete(fut)
        send_fut.result()
        msg.release().result()
        assert msg2.data == b'hello'
        aio_loop.run_until_complete(msg2.release())
    finally:
        loop = a.loop
        a.stop()
        queue.stop()
        # The uv_timer_t used in request, might be released after stopping the
        # chirp instances since it happens in a call_soon. When the loop
        # stopped we know all handles have been released.
        loop.stop()
        loop = None
        fut = None
        msg = None
        msg2 = None
        assert len(gc.get_referrers(a)) == 1 + ref_count_offset


def test_initialize(loop, config):
    """test_initialize."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    aio_loop = asyncio.get_event_loop()
    a = Chirp(loop, config, aio_loop)
    a.stop()


def test_recv_msg(config, sender, message):
    """test_recv_msg."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    aio_loop = asyncio.get_event_loop()
    fut = asyncio.Future()
    res = []

    class MyChirp(Chirp):
        async def handler(self, msg):
            res.append(msg)
            fut.set_result(0)

    a = MyChirp(sender.loop, config, aio_loop)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message)
    aio_loop.run_until_complete(fut)
    time.sleep(0.1)
    assert res[0].data == b'hello'
    assert res[0]._msg_t is None
    a.stop()


def test_ignore_msg(config, sender, message):
    """test_ignore_msg."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = True
    aio_loop = asyncio.get_event_loop()
    fut = asyncio.Future()

    class MyChirp(Chirp):
        pass

    a = MyChirp(sender.loop, config, aio_loop)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    fut = sender.send(message)
    aio_loop.run_until_complete(asyncio.wrap_future(fut))
    a.stop()


def test_recv_await_release(config, sender, message):
    """test_recv_msg."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = False
    aio_loop = asyncio.get_event_loop()
    fut = asyncio.Future()
    res = []

    class MyChirp(Chirp):
        async def handler(self, msg):
            res.append(msg)
            await msg.release()
            fut.set_result(0)

    a = MyChirp(sender.loop, config, aio_loop)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    sender.send(message)
    aio_loop.run_until_complete(fut)
    assert res[0].data == b'hello'
    assert res[0]._msg_t is None
    a.stop()


def test_echo(config, queue, message):
    """test_echo."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = False
    config.SYNCHRONOUS = False
    aio_loop = asyncio.get_event_loop()
    fut = asyncio.Future()

    class MyChirp(Chirp):
        async def handler(self, msg):
            try:
                await self.send(msg)
                fut.set_result(0)
            except Exception as e:
                fut.set_exception(e)
            finally:
                await msg.release()

    a = MyChirp(queue.loop, config, aio_loop)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    queue.send(message)
    aio_loop.run_until_complete(fut)
    msg = queue.get()
    msg.release().result()
    assert msg.data == b'hello'
    assert msg._msg_t is None
    a.stop()


def test_echo_timeout(config, queue, message):
    """test_echo_timeout."""
    config = Config()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.AUTO_RELEASE = False
    config.TIMEOUT = 1
    aio_loop = asyncio.get_event_loop()
    fut = asyncio.Future()

    class MyChirp(Chirp):
        async def handler(self, msg):
            exp = None
            try:
                await self.send(msg)
                fut.set_result(0)
            except Exception as e:
                exp = e
            finally:
                await msg.release()
                fut.set_exception(exp)

    a = MyChirp(queue.loop, config, aio_loop)
    message.data = b'hello'
    message.address = "127.0.0.1"
    message.port = config.PORT
    queue.send(message)
    with pytest.raises(TimeoutError):
        aio_loop.run_until_complete(fut)
    msg = queue.get()
    msg.release().result()
    assert msg.data == b'hello'
    assert msg._msg_t is None
    a.stop()
