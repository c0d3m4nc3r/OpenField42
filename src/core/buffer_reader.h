#pragma once

#include <cstring>

class BufferReader
{
public:
    BufferReader(const char* data, std::size_t size)
        : _data(data), _size(size), _cursor(0) {}

    BufferReader(const std::vector<char>& buffer)
        : _data(buffer.data()), _size(buffer.size()), _cursor(0) {}

    template <typename T>
    T read()
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be POD/trivially copyable");
        
        if (_cursor + sizeof(T) > _size) {
            throw std::out_of_range("BufferReader: Read past end of buffer");
        }

        T value;
        std::memcpy(&value, _data + _cursor, sizeof(T));
        _cursor += sizeof(T);
        return value;
    }

    std::string readString(std::size_t length)
    {
        if (_cursor + length > _size) {
            throw std::out_of_range("BufferReader: Read string past end of buffer");
        }

        std::string str(_data + _cursor, length);
        _cursor += length;
        return str;
    }

    void readBytes(void* dest, std::size_t count)
    {
        if (_cursor + count > _size) {
            throw std::out_of_range("BufferReader: Read bytes past end of buffer");
        }

        std::memcpy(dest, _data + _cursor, count);
        _cursor += count;
    }

    void seek(std::size_t position)
    {
        if (position > _size) {
            throw std::out_of_range("BufferReader: Seek out of bounds");
        }
        _cursor = position;
    }

    void skip(std::size_t bytes)
    {
        seek(_cursor + bytes);
    }

    [[nodiscard]] std::size_t tell() const { return _cursor; }
    [[nodiscard]] std::size_t size() const { return _size; }
    [[nodiscard]] std::size_t remaining() const { return _size - _cursor; }
    [[nodiscard]] bool eof() const { return _cursor >= _size; }

private:
    const char* _data;
    std::size_t _size;
    std::size_t _cursor{0};
};