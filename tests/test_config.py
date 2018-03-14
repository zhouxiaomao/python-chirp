"""Config tests."""
import pytest
from hypothesis import given, assume
from hypothesis.strategies import characters

from libchirp import Config


def test_port_limits(config):
    """test_port_limits."""
    config.PORT = 34
    with pytest.raises(OverflowError):
        config.PORT = 70000


def test_bind_attributes(config):
    """test_bind_attributes."""
    assert config.BIND_V4 == "0.0.0.0"
    assert config.BIND_V6 == "::"
    config.BIND_V4 = "127.0.0.1"
    assert config.BIND_V4 == "127.0.0.1"
    config.BIND_V6 = "1::"
    assert config.BIND_V6 == "1::"
    config.BIND_V6 = "1:0:0::"
    # Always return compressed representation
    assert config.BIND_V6 == "1::"


def test_bools(config):
    """test_bools."""
    for name in Config._bools:
        setattr(config, name, True)
        assert getattr(config, name) is True
        setattr(config, name, False)
        assert getattr(config, name) is False


@given(characters(min_codepoint=1))
def test_strings(config, text):
    """test_strings."""
    try:
        config.CERT_CHAIN_PEM = text
    except UnicodeEncodeError:
        assume(False)
        return
    assert config.CERT_CHAIN_PEM == text


def test_sealed(config):
    """test_sealed."""
    assert config.AUTO_RELEASE is True
    config.AUTO_RELEASE = False
    assert config.AUTO_RELEASE is False
    config._sealed = True
    with pytest.raises(RuntimeError):
        config.AUTO_RELEASE = True
    assert config.AUTO_RELEASE is False
    with pytest.raises(RuntimeError):
        config.BIND_V4 = "0.0.0.0"
