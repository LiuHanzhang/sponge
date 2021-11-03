#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::_update_ackno() {
    _abso_ackno = stream_out().bytes_written() + 1;  // abso ackno = stream index + 1
    if (stream_out().input_ended())
        _abso_ackno++;  // FIN takes on byte in seqno
}

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!_syn_recv && !seg.header().syn)
        return;  // If the connection has not been established, do nothing
    if (_syn_recv && seg.header().syn)
        return;  // Connection established. Refuse another SYN
    if (stream_out().input_ended())
        return;                     // Connection closed. Refuse new segments
    if (seg.header().syn) {         // If syn bit set
        _isn = seg.header().seqno;  // isn is syn segment's seqno
        _syn_recv = true;
    }
    _update_ackno();
    uint64_t abso_seqno = unwrap(seg.header().seqno, _isn, _abso_ackno);  // abso seqno of this incoming segment
    // Now write seg payload to reassembler if connection established
    if (_syn_recv) {
        // stream index = abso seqno - 1, except for SYN segment, whose stream_idx can be treated as zero
        // NOTE: SYN segment cann't carry payload, but it can still set FIN bit
        // However, based on test case, SYN segment seems to be allowed to carry payload?
        // NOTE: If the abso seqno == 0 for a none SYN segment, the abso_seqno - 1 == UINT64_MAX,
        // which overflows window and cannot be written.
        uint64_t stream_idx = seg.header().syn ? 0 : abso_seqno - 1;
        _reassembler.push_substring(seg.payload().copy(), stream_idx, seg.header().fin);
    }
    _update_ackno();
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_syn_recv)
        return wrap(_abso_ackno, _isn);
    else
        return nullopt;
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }