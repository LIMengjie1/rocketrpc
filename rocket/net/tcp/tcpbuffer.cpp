#include "tcpbuffer.h"
#include "log.h"
#include <cstddef>
#include <cstring>
#include <vector>

namespace rocket {

TcpBuffer::TcpBuffer(int size) {
    m_buffer.resize(size);
}

TcpBuffer::~TcpBuffer() {}

int TcpBuffer::readAble() {
    return m_write_index - m_read_index;
}

int TcpBuffer::writeAble() {
    return m_buffer.size() - m_write_index;
}

void TcpBuffer::writeToBuffer(const char* buf, int size) {
    if (size > writeAble()) {
        int new_size = 1.5 * (m_write_index + size);
        resizeBuffer(new_size);
    }
    memcpy(&m_buffer[m_write_index], buf, size);
}

void TcpBuffer::resizeBuffer(int new_size) {
    vector<char> tmp(new_size);
    int count = std::min(new_size, readAble());
    memcpy(&tmp[0], &m_buffer[m_read_index], count);
    m_buffer.swap(tmp);

    m_read_index = 0;
    m_write_index = m_read_index + count;
}

void TcpBuffer::readToBuffer(vector<char>& read, int size) {
    if (readAble() == 0) return;

    int read_size = readAble() > size ? size : readAble(); 

    vector<char> tmp(read_size);

    memcpy(&tmp[0], &m_buffer[m_read_index], read_size);

    read.swap(tmp);
    m_read_index += read_size;
    adjustBuffer();
}

void TcpBuffer::adjustBuffer() {
    if (m_read_index > m_buffer.size() / 3) return;;

    vector<char> buffer(m_buffer.size());

    int count = readAble();
    memcpy(&buffer[0], &m_buffer[m_read_index], count);

    m_buffer = std::move(buffer);
    m_read_index = 0;
    m_write_index = m_read_index + count;
}

void TcpBuffer::moveReadIndex(int size) {
    size_t j = m_read_index + size;
    if(j >= m_buffer.size()) {
        ERRORLOG("movereadindex error, invalid size %d, old read index:%d, buffer size:%d", size, m_read_index, m_buffer.size());
    }
    m_read_index = j;
    adjustBuffer();
}

void TcpBuffer::moveWriteIndex(int size) {
    size_t j = m_write_index + size;
    if(j >= m_buffer.size()) {
        ERRORLOG("move write index error, invalid size %d, old read index:%d, buffer size:%d", size, m_read_index, m_buffer.size());
    }
    m_write_index = j;
    adjustBuffer();
}

}