"""Basic tests."""
import libchirp


def test_bind():
    """Test if libchirp binding works."""
    assert libchirp.lib.CH_SUCCESS == 0
