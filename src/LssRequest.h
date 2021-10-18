//
// Created by guru on 9/24/21.
//

#ifndef LSS_BUS_REQUEST_H
#define LSS_BUS_REQUEST_H

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <stdint.h>

#include "LssCommand.h"

namespace lss {

  #define MAX_ARGS 8

  typedef int32_t cmd_arg_t;

  typedef union {
    struct {
      uint16_t parsed : 1;
      uint16_t reply : 1;

      // Set if this request will be followed by more requests in a group.
      uint16_t continuation : 1;

      // Set if this request specified an ID.
      // Set on the first request in a group, or inside a group when a new
      // servo ID is addressed.
      uint16_t addressed : 1;

      // Set on the last request in a group.
      uint16_t terminal : 1;
    };
    uint16_t value;
  } RequestFlags;

  class Request {
  public:
    Request()
    : id(0), command(command::Unknown), flags({ .value =  0 }),
          nargs(0), args{0}
    {}

    Request(uint8_t _id, command::ID _command)
    : id(_id), command(_command), flags({ .addressed =  true }),
          nargs(0), args{0}
    {}

    Request(uint8_t _id, command::ID _command, cmd_arg_t arg1)
    : id(_id), command(_command), flags({ .addressed =  true }),
          nargs(1), args{arg1,0}
    {}

    Request(uint8_t _id, command::ID _command, cmd_arg_t arg1, cmd_arg_t arg2)
    : id(_id), command(_command), flags({ .addressed =  true }),
          nargs(2), args{arg1, arg2,0}
    {}

    Request(uint8_t _id, command::ID _command,
            cmd_arg_t arg1, cmd_arg_t arg2, cmd_arg_t arg3)
    : id(_id), command(_command), flags({ .addressed =  true }),
          nargs(3), args{arg1, arg2, arg3,0}
    {}

    Request(uint8_t _id, command::ID _command,
            std::initializer_list<cmd_arg_t> args_list)
    : id(_id), command(_command), flags({ .addressed =  true }),
          nargs(std::min((size_t)MAX_ARGS, args_list.size())), args{0}
    {
      auto al_itr = args_list.begin();
      for(int i=0; i < nargs; i++)
        args[i] = *al_itr++;

    }

    // We dont have any complicated data members,
    // so we can use the default compiler generated move
    // and copy constructors.
    Request(Request && move) = default;
    Request(const Request& copy) = default;
    Request& operator=(Request && move) = default;
    Request& operator=(const Request & copy) = default;

    static Request query(const Request& from_ref) {
        // todo: possibly we should convert the command ID to a Query type command?
        return { from_ref.id, from_ref.command };
    }

    void clear() {
      id = 0;
      command = command::Unknown;
      flags.value = 0;
      nargs = 0;
      memset(args, 0, sizeof(args));
    }

    uint8_t id;
    command::ID command;
    RequestFlags flags;

    // arguments in or out
    uint8_t nargs;
    cmd_arg_t args[MAX_ARGS];
  };

} // ns: Lss
#endif // LSS_BUS_REQUEST_H
