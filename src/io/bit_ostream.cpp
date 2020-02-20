#include <tdc/io/bit_ostream.hpp>

using namespace tdc::io;

void BitOStream::reset() {
    m_next = 0;
    m_cursor = MSB;
}

void BitOStream::write_next() {
    m_stream->put(char(m_next));
    reset();
}

BitOStream::BitOStream(std::ostream& stream) : m_stream(&stream) {
    reset();
}

BitOStream::~BitOStream() {
    uint8_t set_bits = MSB - m_cursor; // will only be in range 0 to 7
    if(m_cursor >= 2) {
        // if there are at least 3 bits free in the byte buffer,
        // write them into the cursor at the last 3 bit positions
        m_next |= set_bits;
        write_next();
    } else {
        // else write out the byte, and write the length into the
        // last 3 bit positions of the next byte
        write_next();
        m_next = set_bits;
        write_next();
    }
}

void BitOStream::write_bit(const bool b) {
    m_next |= (b << m_cursor);

    --m_cursor;
    if(m_cursor < 0) {
        write_next();
    }

    ++m_bits_written;
}
