#pragma once

#include <cstddef>
#include <cstdint>

template<class type>
class buffer {
public:
  buffer() 
    : _size(0)
    , _data(nullptr)
  {}

  explicit buffer(size_t size)
    : _size(size)
    , _data(new type[size])
  {
    memset(_data, 0, _size);
  }

  explicit buffer(const void* stuff, size_t size) 
    : buffer(size)
  {
    memcpy(_data, stuff, size);
  }

  buffer(const buffer& other)
    : _size(0)
    , _data(nullptr)
  {
    if (other._data != nullptr && other._size > 0) {
      _size = other._size;
      _data = new type[_size];

      memcpy(_data, other._data, _size);
    }
  }

  buffer& operator=(const buffer& other) {
    if (this != &other) {
      if (_data != nullptr && _size > 0)
        delete[] _data;

      _data = nullptr;
      _size = 0;

      if (other._data != nullptr && other._size > 0) {
        _size = other._size;
        _data = new type[_size];

        memcpy(_data, other._data, _size);
      }
    }
    return *this;
  }

  buffer(buffer&& other)
    : _size(0)
    , _data(nullptr)
  {
    _data = other._data;
    _size = other._size;

    other._data = nullptr;
    other._size = 0;
  }

  buffer& operator=(buffer&& other) {
    if (this != &other) {
      if (_data != nullptr)
        delete[] _data;

      _data = other._data;
      _size = other._size;

      other._data = nullptr;
      other._size = 0;
    }
    return *this;
  }
  

  ~buffer() {
    if (_data != nullptr && _size > 0)
      delete[] _data;
  }

  type* data() {
    return _data;
  }

  const type* data() const {
    return _data;
  }

  type &operator[](int index) {
    return _data[index];
  }

  const type &operator[](int index) const {
    return _data[index];
  }

  size_t size() const {
    return _size;
  }

  const char* c_str() const {
    return reinterpret_cast<const char*>(_data);
  }

private:
  size_t _size;
  type* _data;
};