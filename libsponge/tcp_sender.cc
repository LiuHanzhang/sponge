#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void Timer::start(const size_t retx_timeout) {
    _rto = retx_timeout;
    _elapsed_time = 0;
    _ticking = true;
}

bool Timer::ticktock(const size_t ms_since_last_tick) {
    if (!_ticking)
        return false;
    _elapsed_time += ms_since_last_tick;
    return _elapsed_time >= _rto;
}

void fill_segment(TCPSegment &seg,
                  const uint64_t abso_seqno,
                  const WrappingInt32 &isn,
                  bool syn,
                  bool fin,
                  string &data) {
    seg.header().seqno = wrap(abso_seqno, isn);
    seg.header().syn = syn;
    seg.header().fin = fin;
    if (data.length() > 0)
        seg.payload() = Buffer(move(data));
}

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _outgoing_bytes; }

void TCPSender::fill_window() {
    if (!_syn_sent) {
        // waiting for stream to begin (syn not sent)
        TCPSegment seg;
        string data;  // empty string, no payload in SYN segment
        fill_segment(seg, _next_seqno, _isn, true, false, data);
        _segments_out.push(seg);
        _syn_sent = true;
        _outgoing_segments.push(seg);
        _outgoing_bytes += seg.length_in_sequence_space();
        if (!_timer.ticking())
            _timer.start(_retransmission_timeout);
        _next_seqno += seg.length_in_sequence_space();
        return;
    }
    if (!_syn_acked || _fin_sent)  // Stream not ongoing
        return;
    // syn sent, acked, and fin not sent (stream ongoing)
    while (_next_seqno < _ackno + _win_size && !_fin_sent) {
        size_t read_len = MIN3(_ackno + _win_size - _next_seqno, _stream.buffer_size(), TCPConfig::MAX_PAYLOAD_SIZE);
        bool eof = _stream.eof();
        if (read_len == 0 && !eof)  // If read_len == 0 is not caused by eof
            break;
        string data = _stream.read(read_len);  // NOTE: If eof, this data might be empty string
        eof = _stream.eof();                   // Does reading data reach eof?
        // NOTE: Don't add FIN if this would make the segment exceed the receiver's window
        if (_ackno + _win_size < _next_seqno + read_len + 1)
            eof = false;
        TCPSegment seg;
        fill_segment(seg, _next_seqno, _isn, false, eof, data);
        _segments_out.push(seg);
        _outgoing_segments.push(seg);
        _outgoing_bytes += seg.length_in_sequence_space();
        _fin_sent = eof;
        if (!_timer.ticking())
            _timer.start(_retransmission_timeout);
        _next_seqno += seg.length_in_sequence_space();
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    if (!_syn_sent)
        return;
    uint64_t abso_ackno = unwrap(ackno, _isn, _next_seqno);
    if (abso_ackno > _next_seqno)
        return;  // NOTE: Impossible ackno (beyond next seqno) is ignored. Thanks to test case in send_ack.cc
    _ackno = MAX(abso_ackno, _ackno);
    bool ack_new_data = false;
    while (!_outgoing_segments.empty()) {
        TCPSegment first_outgoing_seg = _outgoing_segments.front();
        uint64_t first_outgoing_seg_seqno = unwrap(first_outgoing_seg.header().seqno, _isn, _next_seqno);
        size_t first_outgoing_seg_len = first_outgoing_seg.length_in_sequence_space();
        if (first_outgoing_seg_seqno + first_outgoing_seg_len <= abso_ackno) {
            if (first_outgoing_seg.header().syn)
                _syn_acked = true;
            _outgoing_segments.pop();
            _outgoing_bytes -= first_outgoing_seg_len;
            ack_new_data = true;
        } else
            break;
    }
    if (ack_new_data) {
        _retransmission_timeout = _initial_retransmission_timeout;
        _consecutive_retransmissions = 0;
        if (_outgoing_segments.empty())
            _timer.stop();
        else
            _timer.start(_retransmission_timeout);
    }
    _win_size = window_size == 0 ? 1 : window_size;
    _win_zero = (window_size == 0);
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer.ticktock(ms_since_last_tick))  // If timer don't goes off
        return;
    _segments_out.push(_outgoing_segments.front());
    if (!_win_zero) {
        _consecutive_retransmissions++;
        _retransmission_timeout *= 2;
    }
    _timer.start(_retransmission_timeout);
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    string data;
    fill_segment(seg, _next_seqno, _isn, false, false, data);
    _segments_out.push(seg);
}
