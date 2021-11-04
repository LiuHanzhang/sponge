#ifndef SPONGE_LIBSPONGE_TCP_SENDER_HH
#define SPONGE_LIBSPONGE_TCP_SENDER_HH

#include "byte_stream.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"

#include <functional>
#include <queue>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#define MIN3(a, b, c) ((MIN((a), (b))) < (c) ? (MIN((a), (b))) : (c))
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

//! \brief Timer, a helper class
class Timer {
  private:
    size_t _elapsed_time{0};
    size_t _rto{0};
    //! If the timer is running
    bool _ticking{false};

  public:
    //! \brief (Re)Start the timer. Can restart the timer while it's still running.
    void start(const size_t retx_timeout);

    //! \returns If the timer is running
    bool ticking() const { return _ticking; }

    //! \brief Make the timer stop
    void stop() { _ticking = false; }

    //! \brief Every time TCPSender::tick() is called, TCPSender will call this.
    //! \returns whether the alarm goes off (i.e., timeout occurs)
    bool ticktock(const size_t ms_since_last_tick);
};

//! \brief The "sender" part of a TCP implementation.

//! Accepts a ByteStream, divides it up into segments and sends the
//! segments, keeps track of which segments are still in-flight,
//! maintains the Retransmission Timer, and retransmits in-flight
//! segments if the retransmission timer expires.
class TCPSender {
  private:
    //! our initial sequence number, the number for our SYN.
    WrappingInt32 _isn;

    //! outbound queue of segments that the TCPSender wants sent
    std::queue<TCPSegment> _segments_out{};

    //! retransmission timer for the connection
    unsigned int _initial_retransmission_timeout;
    unsigned int _retransmission_timeout;
    //! Number of consecutive retransmissions
    unsigned int _consecutive_retransmissions{0};

    //! outgoing stream of bytes that have not yet been sent
    ByteStream _stream;

    //! the (absolute) sequence number for the next byte to be sent
    uint64_t _next_seqno{0};

    //! (absolute) ackno and window size reported by receiver
    //! \note _win_size always >= 1, because we treat zero window size as one
    //! However, in terms of retransmission, we cann't treat zero window size as one
    //! So we need to record if window size reported by receiver is actually zero
    uint64_t _ackno{0};
    uint16_t _win_size{1};
    bool _win_zero{false};

    //! queue of outgoing (sent yet not acked) segments. FIFO
    std::queue<TCPSegment> _outgoing_segments{};
    //! How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t _outgoing_bytes{0};

    //! A timer as explained in lab document
    Timer _timer{};

    //! Connection management variables
    //! \note These variables represent sender's state in another way
    //! See tcp_helpers/tcp_state.hh
    bool _syn_sent{false};
    bool _syn_acked{false};
    bool _fin_sent{false};

  public:
    //! Initialize a TCPSender
    TCPSender(const size_t capacity = TCPConfig::DEFAULT_CAPACITY,
              const uint16_t retx_timeout = TCPConfig::TIMEOUT_DFLT,
              const std::optional<WrappingInt32> fixed_isn = {});

    //! \name "Input" interface for the writer
    //!@{
    ByteStream &stream_in() { return _stream; }
    const ByteStream &stream_in() const { return _stream; }
    //!@}

    //! \name Methods that can cause the TCPSender to send a segment
    //!@{

    //! \brief A new acknowledgment was received
    void ack_received(const WrappingInt32 ackno, const uint16_t window_size);

    //! \brief Generate an empty-payload segment (useful for creating empty ACK segments)
    void send_empty_segment();

    //! \brief create and send segments to fill as much of the window as possible
    void fill_window();

    //! \brief Notifies the TCPSender of the passage of time
    void tick(const size_t ms_since_last_tick);
    //!@}

    //! \name Accessors
    //!@{

    //! \brief How many sequence numbers are occupied by segments sent but not yet acknowledged?
    //! \note count is in "sequence space," i.e. SYN and FIN each count for one byte
    //! (see TCPSegment::length_in_sequence_space())
    size_t bytes_in_flight() const;

    //! \brief Number of consecutive retransmissions that have occurred in a row
    unsigned int consecutive_retransmissions() const;

    //! \brief TCPSegments that the TCPSender has enqueued for transmission.
    //! \note These must be dequeued and sent by the TCPConnection,
    //! which will need to fill in the fields that are set by the TCPReceiver
    //! (ackno and window size) before sending.
    std::queue<TCPSegment> &segments_out() { return _segments_out; }
    //!@}

    //! \name What is the next sequence number? (used for testing)
    //!@{

    //! \brief absolute seqno for the next byte to be sent
    uint64_t next_seqno_absolute() const { return _next_seqno; }

    //! \brief relative seqno for the next byte to be sent
    WrappingInt32 next_seqno() const { return wrap(_next_seqno, _isn); }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_TCP_SENDER_HH
