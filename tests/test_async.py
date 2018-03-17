"""Queue tests."""

import asyncio
import pytest
import time

from libchirp.asyncio import Chirp, Config


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
            pass

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
