"""Message tests."""
import pytest
from hypothesis import given
from hypothesis.strategies import binary, sampled_from, integers

from libchirp import lib
from libchirp import MessageThread as Message


def test_init(message):
    """test_init."""
    assert len(message.identity) == lib.CH_ID_SIZE
    assert message.identity != b'\0' * lib.CH_ID_SIZE
    assert message.serial == 0
    assert len(message.header) == 0
    assert len(message.data) == 0
    assert message.address == "0.0.0.0"
    assert message.port == 0
    assert message.remote_identity == b'\0' * lib.CH_ID_SIZE
    assert message.has_slot is False


@given(
    binary(),
    binary(),
    sampled_from(
        ("1::", "127.0.0.1", "192.168.1.1", "1:4:4::")
    ),
    integers(min_value=0, max_value=2**16)
)
def test_msg_roundtrip(message, header, data, ip, port):
    """test_msg_roundtrip."""
    old_id = message.identity
    message.header = header
    message.data = data
    message.address = ip
    message.port = port
    message._copy_to_c()
    msg2 = Message(message._msg_t)
    # Identity is read-only but must stay the same
    assert msg2.identity == old_id
    assert msg2.header == header
    assert msg2.data == data
    assert msg2.address == ip
    assert msg2.port == port


def test_release_does_nothing(message):
    """test_release_does_nothing."""
    # With the message API only we can't test release_slot(), so we assure that
    # it at least does nothing.
    id_msg = id(message._msg_t)
    message.release_slot().result() is None
    assert id_msg == id(message._msg_t)


def test_header_bad_type(message):
    """test_header_bad_type."""
    with pytest.raises(AssertionError):
        message.header = "bla"


def test_data_bad_type(message):
    """test_data_bad_type."""
    with pytest.raises(AssertionError):
        message.data = "bla"


def test_address_bad_format(message):
    """test_address_bad_format."""
    with pytest.raises(ValueError):
        message.address = "127.0"


def test_port_bad_range(message):
    """test_port_bad_range."""
    with pytest.raises(AssertionError):
        message.port = -1
    with pytest.raises(AssertionError):
        message.port = 65537


def test_identity_quality():
    """test_identity_quality."""
    for _ in range(100):
        msgs = []
        for _ in range(10000):
            msgs.append(Message())
        msg_set = set([msg.identity for msg in msgs])
        assert len(msg_set) == 10000
