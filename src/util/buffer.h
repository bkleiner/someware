#pragma once

#include <cstring>
#include <cstddef>
#include <cstdint>

template<class type>
class buffer {
public:
  constexpr buffer() 
    : _size(0)
    , _data(nullptr)
  {}

  explicit constexpr buffer(size_t size)
    : _size(size)
    , _data(new type[size])
  {
    fill(0);
  }

  explicit constexpr buffer(const void* stuff, size_t size) 
    : buffer(size)
  {
    copy(stuff);
  }

  buffer(const buffer& other)
    : _size(0)
    , _data(nullptr)
  {
    if (other._data != nullptr && other._size > 0) {
      _size = other._size;
      _data = new type[_size];

      copy(other._data);
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

        copy(other._data);
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

  void fill(const type v) {
    memset(_data, v, _size * sizeof(type));
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

  void copy(const void* src) {
    memcpy(_data, src, _size * sizeof(type));
  }
};