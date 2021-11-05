#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPConnection::_send_segments(bool rst) {
    if (rst) {
        TCPSegment seg = _sender.segments_out().front();
        seg.header().rst = true;
        _segments_out.push(seg);
        _sender.segments_out().pop();
        return;
    }

    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }
        seg.header().win = MIN(_receiver.window_size(), UINT16_MAX);
        _segments_out.push(seg);
        _sender.segments_out().pop();
    }
}

bool TCPConnection::_connection_end_prereq() const {
    bool prereq1 = (_receiver.unassembled_bytes() == 0) && _receiver.stream_out().input_ended();
    bool prereq2 = _sender.fin_sent();
    bool prereq3 = (_sender.bytes_in_flight() == 0);
    return prereq1 && prereq2 && prereq3;
}

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _time_since_last_segment_received = 0;
    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _rst = true;
        return;
    }
    _receiver.segment_received(seg);
    if (inbound_stream().input_ended() && !_sender.fin_sent())
        _linger_after_streams_finish = false;
    if (seg.header().ack)
        _sender.ack_received(seg.header().ackno, seg.header().win);
    if (seg.length_in_sequence_space()) {
        // If the incoming segment occupied any sequence numbers, the TCPConnection makes sure that
        // at least one segment is sent in reply, to reflect an update in the ackno and window size.
        _sender.fill_window();
        if (_sender.segments_out().empty())
            _sender.send_empty_segment();
        _send_segments();
    } else if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
               seg.header().seqno == _receiver.ackno().value() - 1) {
        // NOTE: There is one extra special case that you will have to handle in the TCPConnection's
        // segment received() method: responding to a "keep-alive" segment. The peer may choose
        // to send a segment with an invalid sequence number to see if your TCP implementation
        // is still alive (and if so, what your current window is). Your TCPConnection should
        // reply to these "keep-alives" even though they do not occupy any sequence numbers.
        _sender.send_empty_segment();
        _send_segments();
    } else {
        // Else the segment is just a regular empty ack segment
        // Still, this segment may expand receiver's window,
        // so that sender can send new segments
        // _sender.fill_window(); NOTE: Unnecessary, because if the segment is empty ack,
        //                              _sender.ack_received() will have already invoked fill_window()
        _send_segments();
    }

    // NOTE: Examine the illustration of prerequisites, we can find that
    // only after a segment received can these prereqs be fullfilled
    if (_connection_end_prereq()) {
        if (!_linger_after_streams_finish)
            _close = true;
        else
            _linger_timer.start(10 * _cfg.rt_timeout);
    }
}

// FIXME: BUG: If active() is called before connection established, it returns true
bool TCPConnection::active() const { return !_rst && !_close; }

size_t TCPConnection::write(const string &data) {
    size_t write_len = _sender.stream_in().write(data);
    _sender.fill_window();
    _send_segments();
    return write_len;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _time_since_last_segment_received += ms_since_last_tick;

    // If now the connection is lingering
    if (_linger_timer.ticking()) {
        if (_linger_timer.ticktock(ms_since_last_tick)) {
            // If enough linger time elapsed
            _linger_timer.stop();
            _close = true;
        }
        return;
    }
    // Or else now the stream is still ongoing
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions() <= _cfg.MAX_RETX_ATTEMPTS)
        // If not exceed MAX_RETX_ATTEMPTS, retransmit
        _send_segments();
    else {
        // Abort the connection, sent RST
        while (!_sender.segments_out().empty())
            _sender.segments_out().pop();
        _sender.send_empty_segment();
        _send_segments(true);
        _rst = true;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();  // If eof, fill_window is responsible for sending FIN
    _send_segments();
}

void TCPConnection::connect() {
    _sender.fill_window();  // If connection close, fill_window is responsible for sending SYN
    _send_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            while (!_sender.segments_out().empty())
                _sender.segments_out().pop();
            _sender.send_empty_segment();
            _send_segments(true);
            _rst = true;
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
