#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void UnAssembled::insert(const string &data, const size_t index) {
    if (_buffer.begin() == _buffer.end()) {
        // Deal with the special case: empty list
        _buffer.insert(_buffer.begin(), pair<size_t, string>(index, data));
        _size += data.length();
        return;
    }

    list<pair<size_t, string>>::iterator inserted_iter;   // The inserted/updated substring iterator
    list<pair<size_t, string>>::iterator prev, next;      // prev and next iterator of the inserted substring
    const size_t inserted_right = index + data.length();  // The first byte index after inserted substring
    if (index < _buffer.begin()->first) {
        // If the new substring index is smaller than any buffered unordered ones
        // Insert it to the begin of the list
        inserted_iter = _buffer.insert(_buffer.begin(), pair<size_t, string>(index, data));
        _size += data.length();
        next = inserted_iter;
        ++next;
    } else {
        for (prev = _buffer.begin(), next = _buffer.begin(), ++next; next != _buffer.end();
             ++prev, ++next)  // Find the insert position
            if (prev->first <= index && index < next->first)
                break;                                                  // Find the insert position
        const size_t prev_right = prev->first + prev->second.length();  // The first byte index after prev substring
        if (index > prev_right) {
            // If the new substring does not overlap with previous one
            inserted_iter = _buffer.insert(next, pair<size_t, string>(index, data));  // insert it here
            _size += data.length();
        } else if (inserted_right > prev_right) {
            // If overlap, update the prev substring as needed
            size_t start = prev_right - index;
            string trunc(data, start, string::npos);  // remove the overlapped part of new substring
            prev->second.append(trunc);               // update the prev substring
            _size += trunc.length();
            inserted_iter = prev;
        }  // else if the new substring is included in prev string, do nothing
    }
    // Now that we've inserted the new substring, it may overlap with its successors.
    // Merge the successors iteratively into the list, until they don't overlap with the inserted one
    // The merging logic is basically the same as merging the new substring with its precursor
    while (next != _buffer.end()) {
        if (next->first > inserted_right)  // The first successor that do not overlap with the new inserted one
            break;                         // Then the subsequent succesors surely do not overlap with the new one
        else {                             // If this successor overlap with the new inserted substring
            // update the inserted substring as needed
            if (next->first + next->second.length() > inserted_right) {
                size_t start = inserted_right - next->first;
                string trunc(next->second, start, string::npos);
                inserted_iter->second.append(trunc);
                _size += trunc.length();
            }
            // then delete the subsequent substring which overlaps with the inserted one
            _size -= next->second.length();
            next = _buffer.erase(next);  // Delete this successor, and next point to next successor
        }
    }
}

string UnAssembled::pop(const size_t seqno) {
    if (_buffer.begin() != _buffer.end() && _buffer.front().first == seqno) {
        string ordered_substring(_buffer.front().second);
        _buffer.pop_front();
        _size -= ordered_substring.length();
        return ordered_substring;
    } else
        return string();
}

StreamReassembler::StreamReassembler(const size_t capacity)
    : _buffer(), _output(capacity), _capacity(capacity), _seqno(0), _eof_idx(-1) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t new_right = index + data.length();  // The first byte index after the new substring
    if (new_right <= _seqno) {
        // If the new substring has already been assembled
        // Still, this new substring may carry eof signal
        if (new_right == _seqno && eof) {
            _eof_idx = new_right;
            _output.end_input();
        }
        return;
    }

    // Handling Capacity Overflow

    // The start index (relative to the beginning of the substring, not the entire stream)
    // and length of the new substring
    size_t start = 0, len = data.length();
    if (index < _seqno && new_right > _seqno) {
        // If the new substring overlaps with assembled ones, truncate assembled bytes
        start = _seqno - index;
        len = data.length() - start;
    }
    bool truncated = false;
    if ((new_right - _seqno) + _output.buffer_size() > _capacity) {
        // If capacity overflows, truncate overflowed bytes
        len -= (new_right - _seqno) + _output.buffer_size() - _capacity;
        new_right = index + start +
                    len;  // TRAP: Different from what I wrote in 20fa version. But this doesn't introduce any bug
        truncated = true;
    }
    string trunc(data, start, len);
    // push the new truncated substring into unassembled buffer
    // and have it merge with other unordered ones
    _buffer.insert(trunc, index + start);
    string ordered_substring = _buffer.pop(_seqno);
    if (eof && !truncated)  // If data is truncated, eof doesn't count
        _eof_idx = MIN(new_right, _eof_idx);
    if (_eof_idx <= _seqno + ordered_substring.length()) {  // reach eof
        ordered_substring = ordered_substring.substr(0, _eof_idx - _seqno);
        _output.write(ordered_substring);
        _output.end_input();
    } else
        _output.write(ordered_substring);
    _seqno += ordered_substring.length();
}

size_t StreamReassembler::unassembled_bytes() const { return _buffer.size(); }

bool StreamReassembler::empty() const { return _buffer.size() == 0; }
