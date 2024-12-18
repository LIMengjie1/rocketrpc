#pragma once
#include "god.h"
#include <memory>
#include <vector>

namespace rocket {
class TcpBuffer {
public:
    using s_ptr = std::shared_ptr<TcpBuffer>;
    TcpBuffer(int size);

    ~TcpBuffer();

    int readAble();

    int writeAble();

    int readIndex() const {
        return m_read_index;
    }

    int writeIndex() const {
        return m_write_index;
    }

    void writeToBuffer(const char* buf, int size);

    void readToBuffer(std::vector<char>&, int);

    void resizeBuffer(int);

    void adjustBuffer();

    void moveReadIndex(int);

    void moveWriteIndex(int);
    vector<char>& getBuffer() {
        return m_buffer;
    }
private:
    int m_read_index = 0;
    int m_write_index = 0;
    vector<char> m_buffer;
};

}