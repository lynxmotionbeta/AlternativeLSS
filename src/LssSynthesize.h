//
// Created by guru on 10/5/21.
//

#ifndef LSS_BUS_LSSSYNTHESIZE_H
#define LSS_BUS_LSSSYNTHESIZE_H

#include "LssRequest.h"
#include <vector>

namespace lss {

class DeviceIndex;

std::vector<Request> generate_slot_config(DeviceIndex& index);

template<int N>
class Synthesize
{
public:
  Synthesize()
  : _head(buffer)
  {}

  const char* operator()(const Request& one) {
    const char* head = _head;
    write(one, RequestFlags {{ .addressed = true, .terminal = true }});
    if(_head > buffer + sizeof(buffer)/2)
        _head = buffer;
    return head;
  }

  inline const char* operator()(const Request* begin, size_t count) {
      return operator()(begin, begin + count);
  }

  inline const char* operator()(std::vector<Request>& requests) {
    return operator()(&*requests.begin(), &*requests.end());
  }

  inline const char* operator()(std::vector<Request>::const_iterator begin,
      std::vector<Request>::const_iterator end) {
    return operator()(&*begin, &*end);
  }

  template<int size>
  inline const char* operator()(Request (&begin)[size]) {
      return operator()(begin, begin + size);
  }

  // synthesize a request that will fulfil the given request objects
  // this doesnt exactly serialize the requests since it won't include arguments
  inline const char* request(const Request& one) {
    const char *head = _head;
    if(one.enable) {
      Request req = Request::query(one);
      req.flags.continuation = false;
      req.flags.terminal = true;
      write(req);
    }
    return head;
  }

  const char* operator()(const Request* begin, const Request* end)
  {
    if(begin >= end)
      return nullptr;

    // skip any disabled requests at the beginning
    while(!begin->enable && begin < end)
      begin++;
    // skip any disabled requests at the end
    while(end > begin && !(end - 1)->enable)
      --end;

    int count = end - begin;
    const char* head = _head;

    // first request
    uint8_t id = begin->id;
    RequestFlags flags = {{
      .continuation = --count > 0,
      .addressed = true
    }};
    flags.terminal = !flags.continuation;
    write(*begin++, flags);

    while(begin < end) {
      --count;
      if(begin->enable) {
        flags = {{
          .continuation = count > 0,
          .addressed = begin->id != id,
          .terminal = count == 0
        }};
        id = begin->id;
        write(*begin++, flags);
      } else
        begin++;
    }

    if(_head > buffer + sizeof(buffer)/2)
      _head = buffer;
    return head;
  }

  // synthesize a request that will fulfil the given request objects
  // this doesnt exactly serialize the requests since it won't include arguments
  inline const char* request(const Request* begin, const Request* end) {
      if(begin >= end)
          return nullptr;

      // skip any disabled requests at the beginning
      while(!begin->enable && begin < end)
        begin++;
      // skip any disabled requests at the end
      while(end > begin && !(end - 1)->enable)
        --end;

      int count = end - begin;
      const char* head = _head;
      Request req;

      // first request
      req = Request::query(*begin++);
      uint8_t id = req.id;
      auto flags = RequestFlags {{
          .continuation = --count > 0,
          .addressed = true
      }};
      flags.terminal = !flags.continuation;
      write(req, flags);

      while(begin < end) {
        --count;
        if(begin->enable) {
          req = Request::query(*begin++);
          flags = {{
              .continuation = count > 0,
              .addressed = begin->id != id,
              .terminal = count == 0
          }};
          id = req.id;
          write(req, flags);
        } else
          begin++;
      }

      if(_head > buffer + sizeof(buffer)/2)
          _head = buffer;
      return head;
  }

  inline const char* request(const Request* begin, size_t count) {
      return request(begin, begin + count);
  }

  template<int size>
  inline const char* request(Request (&begin)[size]) {
      return request(begin, begin + size);
  }

protected:
  char buffer[N];
  char *_head;

  // serializes a uint32 into the output character stream
  void write(uint32_t i)
  {
    if (i > 9)
      write(i / 10);
    append( '0' + i % 10);
  }

  // serializes a uint32 into the output character stream
  void write(int32_t i)
  {
    if(i < 0) {
      append('-');
      return write((uint32_t)-i);
    } else
      return write((uint32_t)i);
  }

  inline void write(const Request& one) {
    return write(one, one.flags);
  }

  void write(const Request& one, RequestFlags flags) {
    if(!one.enable)
      return;
    const char* cmd_s = command::to_string(one.command);
    if(cmd_s == nullptr)
      return;

    if(flags.addressed) {
        append( flags.reply ? '*' : '#' );
      write(one.id);
    }

    while(*cmd_s)
        append(*cmd_s++);

    // synhesize arguments
    if(one.nargs > 0) {
      // first arg
      write(one.args[0]);

      // remaining args
      for(int i=1; i < one.nargs; i++) {
          append(',');
        write(one.args[i]);
      }
    } else if(flags.continuation) {
        append('0');
    }

    if(flags.terminal)
        append('\r');

    *_head = 0;
  }

  inline void append(char c) {
      *_head++ = c;
  }

};

} // ns:Lss
#endif // LSS_BUS_LSSSYNTHESIZE_H
