#pragma once

#include "buffer.h"
#include "math.h"

template<class type>
class ring_buffer {
public:
  ring_buffer(size_t size)
    : _buf(size)
    , _front(0)
    , _count(0)
  {}

  const type* front() const {
    return _buf.data() + _front;
  }

  size_t count() {
    return _count;
  }

  bool full() const {
    return _count == _buf.size();
  }

  bool empty() const {
    return _count == 0;
  }

  bool write(const type& t) {
    if (full()) {
      return false;
    }

    const auto end = (_front + _count) % int32_t(_buf.size());
    _buf[end] = t;
    _count++;
    return true;
  }

  bool read(type* t) {
    if (empty()) {
      return false;
    }

    *t = _buf[_front];
    
    _front = (_front + 1) % int32_t(_buf.size() - 1);
    _count--;
    return true;
  }
/*
  bool consume(int32_t max_size) {
    if (empty()) {
      return false;
    }
    
    const auto count = util::min(max_size, _count);
    for (int32_t i = 0; i < count; i++) {
      _front = (_front + 1) % int32_t(_buf.size() - 1);
      _count--;
    }
    return true;
  }
*/
private:
  buffer<type> _buf;

  int32_t _front;
  int32_t _count;
};
