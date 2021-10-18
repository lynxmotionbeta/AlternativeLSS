//
// Created by guru on 9/25/21.
//

#ifndef LSS_BUS_RINGBUFFER_H
#define LSS_BUS_RINGBUFFER_H

#include "LssRequest.h"

namespace lss {

using index_t = int;
using ref_count_t = uint16_t;

template<int N>
struct Modulus
{
  // todo: requires N is base_2, perhaps do some SFINAE to provide a
  //  non-base2 one or produce an error if not base_2
  constexpr static index_t modulus(index_t i)  {
    // fastest way to get the modulus of indices when the length
    // is a power of 2
    return i & (N - 1);
  }
};

#if 0
typedef enum { Unused, Parsing, Parsed, InUse } RequestState;

template<int N>
class RequestBuffer {
public:
  class Ref {
  protected:
    Ref(RequestBuffer& parent, index_t index, Request* obj)
    : _parent(&parent), _index(index), _obj(obj)
    {
      // RequestBuffer is not thread-safe
      _parent->obtain(index);
    }

    Ref(RequestBuffer& parent, index_t index)
    : Ref(parent, index, parent[_index])
    {}

  public:
    Ref()
      : _parent(nullptr), _index(0), _obj(nullptr)
    {
    }

    // todo: copy and move constructors

    ~Ref() { release(); }

    inline void release() {
      if(_parent) {
        _parent->release(_index);
      }
      _parent = nullptr;
      _obj = nullptr;
      _index = 0;
    }

    inline explicit operator bool() const { return _obj != nullptr; }

    inline const Request* operator->() const { return _obj; }
    inline Request* operator->() { return _obj; }

    inline const Request* get() const { return _obj; }
    inline Request* get() { return _obj; }

    inline const Request& operator*() const { return *_obj; }
    inline Request& operator*() { return *_obj; }

    inline bool next() {
      if(_obj) {
        _obj = _obj->next;
      }
      return _obj != nullptr;
    }


  public:
    RequestBuffer* _parent;
    // the range of elements in this reference object
    index_t _index{};
    // the current referenced object
    Request* _obj{};

    friend class RequestBuffer;
  };


  RequestBuffer()
  : _head(0), _parse_head(0), _tail(0)
  {
  }

  virtual ~RequestBuffer() {
  }

  inline index_t count() const { return _head - _tail; }

  Request* insert() {
    if(count() < N) {   // todo: 1 or 0?
      if(_head > N) {
        // adjust all indices backwards
        _tail -= N;
        _parse_head -= N;
        _head -= N;
      }

      // use the placement new operator to initialize the request object
      // todo: probably we already have constructed request objects on startup
      auto el = operator[](_head);
      if(el) {
        _head++;
        return el;
      }
    }
    return nullptr;
  }

  Ref pop() {
    Request* el;
    //if(_tail == _head || (el = operator[](_tail))->state < Parsed) {
    if(_tail >= _parse_head) {
      // nothing available to pop
      return {};
    }

    Ref ref(*this, _tail, el);
    auto* req = ref.get();
    while(req) {
      // todo: we shouldnt have to do this forever
      req->state = InUse;
      req = req->next;
    }
    return ref;
  }


  inline Request* operator[](index_t idx) {
    return &_buffer[Modulus<N>::modulus(idx)];
  }


protected:
  ref_count_t _refs[N];
  Request _buffer[N];

  index_t _head;         // insert position
  index_t _parse_head;   // first request in the currently parsing group
                         // request
  index_t _tail;         // next request to be returned to the user.
                         // all the related requests will popped off the
                         // ring buffer

  Request* obtain(index_t index) {
    index = Modulus<N>::modulus(index);
    ++_refs[index];
    return _buffer[index];
  }

  void release(index_t index) {
    assert(_refs[index] > 0);
    index = Modulus<N>::modulus(index);
    if(--_refs[index] == 0)
      free(index);
  }

  void free(index_t index) {
    assert(_refs[index] == 0);
    index = Modulus<N>::modulus(index);

    // free up all requests in this group
    auto el = operator[](index);
    while(el) {
      // call the destructor
      el->clear();
      el = el->next;
    }

    // see if we can advance the tail now
    while(_tail < _head
           && (el = operator[](_tail)) && el->state == Unused) {
      _tail++;
    }
  }
};

#endif

} // ns:Lss
#endif // LSS_BUS_RINGBUFFER_H
