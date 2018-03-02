"""Chirp tests."""
from libchirp import ChirpBase, Loop
import gc
import pytest


def test_value_error(loop, config):
    """test_lifecycle."""
    with pytest.raises(ValueError):
        ChirpBase(loop, config)


def test_lifecycle(config):
    """test_lifecycle."""
    loop = Loop()
    loop.run()
    config.DH_PARAMS_PEM = "./tests/dh.pem"
    config.CERT_CHAIN_PEM = "./tests/cert.pem"
    a = ChirpBase(loop, config)
    assert len(gc.get_referrers(a)) > 1
    a.stop()
    assert len(gc.get_referrers(a)) == 1
    assert loop.running
    loop.stop()
    assert len(gc.get_referrers(loop)) == 1
    assert not loop.running


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
