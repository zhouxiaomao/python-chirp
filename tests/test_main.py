"""Basic tests."""
from libchirp import lib, ffi


def test_bind():
    """test_bind."""
    assert lib.CH_SUCCESS == 0


def test_ch_chirp_config_init():
    """test_ch_chirp_config_init."""
    config = ffi.new("ch_config_t *")
    port = config.PORT
    lib.ch_chirp_config_init(config)
    assert port != config.PORT
