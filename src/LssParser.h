//
// Created by guru on 9/24/21.
//

#ifndef LYNXMOTION_LSS_PARSER_H
#define LYNXMOTION_LSS_PARSER_H

#include "LssRequest.h"
#include <cstdint>

namespace Lss {

typedef enum {
  NoError = 0,
  ExpectedID = 1,
  UnknownCommand = 2,
  ExpectedIntValue = 3,
  ExpectedStringValue = 4,
  CorruptPacket = 5,
  TooManyArguments = 6,
  OutOfMemory = 7,
  ParserInternalError
} ParseError;

typedef enum {
  Noise = -1, // will cause a CorruptPacket if within packet processing
  Idle = 0,
  ParseID,
  ParseCommand,
  ParseIntValue,
  ParseStringValue,
  ParseSeperator,
  ParseTerminator,
  ClearRequest,
  ClearLocalRequest,
  IgnorePacket,
  ParsingError
} ParserState;

typedef enum {
  LexerIdle = 0,
  ReadInt = Idle + 1,
  ReadUnsigned,
  ReadIntMain,
  IgnoreInt,
  IgnoreUnsignedInt,
  ReadLexicon
} LexerState;

struct TrieNode;

class Parser {
public:
  Parser();

  // Begin parsing using a single request object.
  // This request is parsed/ready when the parse() function returns true,
  // and the next parse() will reset the single request for the next command.
  void begin(Request& one);

  // Begin parsing an array of requests in sequence.
  // Can be used to parse requests or replies.
  void begin(Request* parse_begin, Request* parse_end);

  // Begin parsing an array of requests in sequence.
  // Can be used to parse requests or replies.
  inline void begin(Request* parse_begin, size_t count) {
    begin(parse_begin, parse_begin + count);
  }

  // Begin parsing from a ring buffer.
  // Sets the current request, and a number of requests to parse which may
  // wrap around the ring buffer.
  void begin(Request* ring_begin, Request* current, Request* ring_end,
             size_t length = -1);

  inline void set_count(size_t cnt) {
    _count = (ssize_t)cnt;
  }

  inline Request* current() { return _current; }

  // todo: Add sync read, means requests are matched to input array
  //   by ID and Command:
  //  - we have to pass in a C array with length, even if there is only 1.
  //  - add parse mode (request/reply) based on reading # or *
  //  - when Command is parsed, if sync is set, we ensure current request
  //    matches parsed command ID else we advance current request until it
  //    either matches or we get to the end of the array in which a Sync
  //    error would be reported.
  // returns true when count is 0 or -1 (ring buffer parsing). This means
  // all given requests have been parsed. If sync is enabled, then some
  // packets may have been skipped and their parse flag would be false.
  bool parse(char c);


protected:
  LexerState lexer_state;
  ParserState parser_state;

  // the current request, must be within {begin, end} buffer range
  Request *_current;

  // {begin, end} is treated as a ring buffer, if this is undesirable
  // then ensure length doesnt wrap around the end
  Request *_begin, *_end;

  // the number of requests to parse
  ssize_t _count;

  // Some states are like sub-routines, they should run once then
  // return to parser_state. If this subroutine is non-zero then it
  // runs before parser_state is run.
  // An example is the ClearRequest state.
  ParserState parser_subroutine_state;

  void reset();
  void lexer_read_int();
  void lexer_read_unsigned();
  void lexer_read_lexicon();
  void set_error(ParseError error);

  // if we are returning a complete request
  // we may have an unprocessed character to keep
  char unprocessed_char;

  // if true, perform no parsing until either new requeset buffer(s)
  // are set or length is set
  bool halt;

  // numeric parsing
  union {
    int32_t i;
    uint32_t u;
    char *s; // todo: probably remove this union and receive string in args as
             // union
  } v;
  struct TrieNode* lexicon_node;
  bool negative; // indicates number is negative
  uint8_t w;     // free variable for the lexer (usually indicates number of
                 // characters swallowed by the lexer)
  void advance_request();
};

} // ns:Lss
#endif // LYNXMOTION_LSS_PARSER_H
