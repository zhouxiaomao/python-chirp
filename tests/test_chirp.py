"""Chirp tests."""
import gc
import platform
import pytest
import time
import os

from libchirp import ChirpBase, Loop, MessageThread, lib

_echo_test = os.path.exists("./echo_test") or os.path.exists("./echo_test.exe")


def test_value_error(loop, config):
    """test_value_error."""
    with pytest.raises(ValueError):
        ChirpBase(loop, config)


def test_too_high_timeout(loop, config):
    """test_too_high_timeout."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    config.TIMEOUT = 1201
    try:
        ChirpBase(loop, config)
    except ValueError as e:
        assert "Config: timeout must be <= 1200." in e.args[0]


def test_lifecycle(config, ref_count_offset):
    """test_lifecycle."""
    loop = Loop()
    loop.run()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = ChirpBase(loop, config)
    try:
        assert a.identity != b'\0' * lib.CH_ID_SIZE
        assert len(gc.get_referrers(a)) > 1 + ref_count_offset
        a.stop()
        assert len(gc.get_referrers(a)) == 1 + ref_count_offset
        assert loop.running
        loop.stop()
        assert len(gc.get_referrers(loop)) == 1 + ref_count_offset
        assert not loop.running
    except BaseException:
        a.stop()
        loop.stop()
        raise


def test_listen_error(loop, config):
    """test_listen_error."""
    try:
        config.DH_PARAMS_PEM = "./tests/dh.pem"
        config.CERT_CHAIN_PEM = "./tests/cert.pem"
        a = ChirpBase(loop, config)
        with pytest.raises(OSError):
            ChirpBase(loop, config)
    finally:
        a.stop()


def test_send_msg_conn_fail(loop, config, message):
    """test_send_msg_conn_fail."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = ChirpBase(loop, config)
    try:
        message.address = "127.0.0.1"
        message.port = 3000
        with pytest.raises(ConnectionError):
            a.send(message).result()
    finally:
        a.stop()


@pytest.mark.skipif(not _echo_test, reason="No echo_test")
def test_send_msg(loop, config, message, echo):
    """test_send_msg."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = ChirpBase(loop, config)
    try:
        message.address = "127.0.0.1"
        message.port = 2993
        a.send(message).result()
    finally:
        a.stop()


@pytest.mark.skipif(not _echo_test, reason="No echo_test")
def test_send_msg_perf(loop, config, echo, capsys):
    """test_send_msg_perf."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = ChirpBase(loop, config)
    try:
        m = []
        for _ in range(100):
            message = MessageThread()
            message.address = "127.0.0.1"
            message.port = 2993
            m.append(message)
        start = time.time()
        for _ in range(100):
            t = []
            for message in m:
                t.append(a.send(message))
            for it in t:
                it.result()
        end = time.time()
        with capsys.disabled():
            print("\n%d msg/s" % (10000 / (end - start)))
    finally:
        a.stop()


def test_send_msg_network_unavailable(loop, config, message):
    """test_send_msg_network_unavailable."""
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    try:
        a = ChirpBase(loop, config)
        message.address = "127.0.0.0"
        message.port = 3000
        if platform.system() == "Linux":
            with pytest.raises(ConnectionError):
                a.send(message).result()
        else:
            with pytest.raises(TimeoutError):
                a.send(message).result()
    finally:
        a.stop()
