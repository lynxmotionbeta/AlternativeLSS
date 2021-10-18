//
// Created by guru on 9/24/21.
//

#include "LssParser.h"
#include <cassert>
#include <malloc.h>


namespace lss {

#ifndef IS_DIGIT
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#endif

#ifndef IS_ALPHA
#define IS_ALPHA(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#endif

#ifndef UPPER
#define UPPER(c) ((c>='a' && c<='z') ? (c+'A'-'a') : c)
#endif


const char* lss_command_lexicon =
"01F5:A14C22D7EEAEFB8GD4HD8IDALDEME2OE8PE8QF6R^01A8S^01BAT^01D4U^01D4W^01DEY^01E2$A07B07D09H09R09S09$*79R02$*28*7A*7C*81*7BA0EB1CE1CF1EG24H26I2AL2EN38O3AP44R46S4C$A07B07D09H09R09S09$*0CR02$*29*0D*0A*19*09*11M02$*1AD03P03$*22C02$*1BM02*13*7ED03M03*34*0B*21D03S03$*12*23B05E05N07P07*35*1DD02*15*1C*17*16H02*42*43N02*18F02$I02$R02$M02$*27O02$*14C03I03$*10S02$*24D03R03$*0E*0FB03E25*36B07C09D0FL11M15P1B$T02$*2EL04P04W04$*01*2D*2CW02$*2BC03T03$*2F*00M02$G02$V02$*2AT02$*08F02$A02$U02$L02$T02$*25C04M06Q06$R02$*84*7DR02$*85A03P19$C02$T02$O02$R02$Y02$O02$F02$F02$S02$E02$T02$*41C02$*82C02*7FT02$*88M02*3B*3FP02$E02$*87E02*3CD02$*77D03M03*38*39D02$*40*80R02*37O02$T02$O02$C02$O02$L02$*06A15B23C23D29E2BF2DG4BH4BI4DL55M5FN67O67P67R77S7DT91V99W9BY9F*44A07B07D09H09R09S09$*4CR02$*76*4D*4F*69*4E*62R03S03*5A*46L02$*73T02*4A*4BM02$*47A04D1AP1A*60C02$T02$O02$R02$Y02$O02$F02$F02$S02$E02$T02$*6B*65C02$*6A*64D02$*50D04P04S06$*63E02$*75*66B05E05N07P07$*71D02*6D*70*6F*6ED04M04S06*5E*45D02$*74*5F*61*49O03R03*48*6CO02$T02$O02$C02$O02$L02$*07I03P05$S02$*67*72D04L04R12*52*54O02$T02$C02*02O02$U02$N02$T02$*04*55C03Q03*5C*5DM03T03*57*59*58T02*5B*56D03R03$*51*53*68D03E05$M02$*3AC05S05T09V09$*1EE02$T02$*86*20*1FC05D09L09R17*32S02$L02$*78*30O02$T02$C02*03O02$U02$N02$T02$*05*31*33P02$D02$A02$T02$E02$*26D03R03$*3D*3E*83==";

typedef int16_t TrieCode;

// this code indicates the lexer could not match the term
#define TERMINAL_CODE (-1)

typedef struct TrieNode {
  char c;             // when c is NULL then code is the return value
  TrieCode code;       // when c is not NULL, code is the offset to the next letter table
} TrieNode;

TrieNode* lss_command_lexicon_root = nullptr;

constexpr unsigned char from_hex_char(char c) {
  return (c < '0')
         ? 0xff  // invalid char
         : (c <= '9')
             ? (c - '0')
             : (c < 'A')
                 ? 0xff  // invalid char
                 : (c < 'G')
                     ? (c - 'A' + 10)
                     : (c < 'a')
                         ? 0xff  // invalid char
                         : (c < 'g')
                             ? (c - 'a' + 10)
                             : 0xff;   // invalid char
}

TrieNode* parse_trie_code(const char* code) {
  const char* p = code;
  int count;

  // assert string is complete with a count header and the == tail
  union {
    unsigned char bytes[2];
    struct {
      unsigned char a: 4;
      unsigned char b: 4;
      unsigned char c: 4;
      unsigned char d: 4;
    };
    uint16_t u16;
    TrieCode code;
  } __attribute__((packed)) u;
  unsigned char a, b, c, d;
  d = from_hex_char(*p++);
  c = from_hex_char(*p++);
  b = from_hex_char(*p++);
  a = from_hex_char(*p++);
  if((*p++ != ':') || ((a|b|c|d)>16)) {
    // invalid header
    return nullptr;
  }
  u.a = a;
  u.b = b;
  u.c = c;
  u.d = d;

  TrieNode* root = (TrieNode*)calloc(u.u16, sizeof(TrieNode));
  TrieNode* end_node = root + u.u16;
  TrieNode* node = root;

  // parse the tokens in the trie code
  while(*p != '=' && node < end_node) {
    char c = *p++;
    if(c == '$') {
      node->c = 0;
      node->code = TERMINAL_CODE;
      node++;
    } else {
      // retrieve the char code
      node->c = (c == '*') ? 0 : c;

      u.u16 = 0;
      if(*p == '^') {
        // read upper byte
        p++;  // swallow ^
        if(16 < (u.d = from_hex_char(*p++)))
          goto fail;
        if(16 < (u.c = from_hex_char(*p++)))
          goto fail;
      }

      // retrieve the integer code
      if(16 < (u.b = from_hex_char(*p++)))
          goto fail;
      if(16 < (u.a = from_hex_char(*p++)))
          goto fail;
      node->code = u.code;
      node++;
    }
  }

  count = node - root;

  // check tail for two equals
  if(*p++ != '=' || *p++ != '=') {
    goto fail;
  }

  return root;
fail:
  return nullptr;
}
// 14 x 1.75 x 0.5

Parser::Parser()
: lexer_state(LexerIdle), parser_state(Idle), parser_subroutine_state(Idle),
  unprocessed_char(0), v({0}), negative(false), w(0)
{
  if(!lss_command_lexicon_root)
    lss_command_lexicon_root = parse_trie_code(lss_command_lexicon);
}

void Parser::begin(Request& one)
{
  _current = _begin = _end = &one;
  _count = -1;
  reset();
}

void Parser::begin(Request* parse_begin, Request* parse_end)
{
  _current = _begin = parse_begin;
  _end = parse_end;
  _count = _end - _begin;
  reset();
}

void Parser::begin(Request* ring_begin, Request* current, Request* ring_end,
           size_t length)
{
  _begin = ring_begin;
  _end = ring_end;
  _current = current;
  _count = length;
  reset();
}

void Parser::reset() {
  lexer_state = LexerIdle;
  parser_state = Idle;
  // ensure the packet is cleared on the next parse call
  parser_subroutine_state = ClearRequest;
  halt = false;
}

void Parser::advance_request() {
  if(_count > 0) {
    Request* prev = _current;

    // advance to start the next request
    _current++;
    if(_current >= _end) {
      _current = _begin;
    }

    if(_count != -1 && --_count == 0) {
      // enter halt state
      // set as a sub-routine so if the user sets new request buffer(s)
      // we can continue where we left off
      // todo: but all begin() functions explicitly reset state (so we cannot
      //  split across group commands)
      halt = true;
      return;
    }

    // todo: sync ID
    _current->id = prev->id;
    _current->flags.reply = prev->flags.reply;
  }
}

void Parser::lexer_read_int()
{
  lexer_state = ReadInt;
  negative = false;
  v.i = 0;
}

void Parser::lexer_read_unsigned()
{
  lexer_state = ReadUnsigned;
  negative = false;
  v.i = 0;
}

void Parser::lexer_read_lexicon()
{
  lexer_state = ReadLexicon;
  lexicon_node = lss_command_lexicon_root;
  negative = false;
}

void Parser::set_error(ParseError error) {
  // todo: we could reply with an error reply here
  lexer_state = LexerIdle;
  parser_state = ParsingError;
  //char s[32];
  //sprintf(s, "E%d:L%dP%d\r", error, parser->lexer_state, parser->parser_state);
  //SERIAL_PutString(s);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wswitch"

//#define UNPROCESSED_RETURN(c) { unprocessed_char = c; return true; }
bool Parser::parse(char c)
{
    if(halt)
      //  ignore incoming data until we are given a request to fill
      return false;

    // check for unprocessed char
    if(unprocessed_char >0) {
      char upc = unprocessed_char;
      unprocessed_char = 0;
      if(parse(upc)) {
        // unprocessed char resulted in a complete request
        unprocessed_char = c; // now save current char
        return true;
      }
    }

    // check if we are currently in a lexer state.
    // characters that terminate lexing state should recursively
    // call receive() to swallow the terminating character
restart_lexer:
    switch(lexer_state) {
        case LexerIdle:
            break;
        case ReadInt:
            if(c == '-') {
                negative = true;
                lexer_state = ReadUnsigned;
                return false;
            }
            lexer_state = ReadUnsigned;
            // fall through
        case ReadUnsigned:
            // read the first digit only
            if(IS_DIGIT(c)) {
                v.u = c - '0';
                lexer_state = ReadIntMain;
                return false;
            } else
                set_error(ExpectedIntValue);
            return false;
        case ReadIntMain:
            if(IS_DIGIT(c)) {
                v.u *= 10;
                v.u += c - '0';
                return false;
            } else {
                lexer_state = LexerIdle;
                if(negative)
                    v.i = -(int32_t)v.u;
                goto restart_parser;
            }
        case IgnoreInt:
            lexer_state = IgnoreUnsignedInt;
            if(c == '-')
                return false; // swallow but ignore
            // fall-through
        case IgnoreUnsignedInt:
            if(!IS_DIGIT(c)) {
                lexer_state = LexerIdle;
                goto restart_parser;
            }
            return false;
        case ReadLexicon: {
            uint16_t C = UPPER(c);
            while (lexicon_node->c != 0) {
                if (lexicon_node->c == C) {
                    // jump to the next table (offset)
                    lexicon_node += lexicon_node->code;
                    return false;// success, wait for next character
                } else
                    lexicon_node++;
            }
            // encountered end of lexer chain
            // parser will now decide if it's a success or failure
            lexer_state = LexerIdle;
            goto restart_parser;
        }
    }

    restart_parser:
      switch(
          // parser_subroutine_state preempts normal state
          (parser_subroutine_state == Idle)
              ? parser_state
              : parser_subroutine_state) {
        case Idle:
            // expect packet start
            if(c == '#' || c == '*') {
            packet_start:
                // initiate a new command
                parser_state = ParseID;
                lexer_read_unsigned();

                // if parser_new_packet() fails (i.e. full command stack)
                // then it resets state to Idle.
                _current->clear();
                _current->flags.value = 0;
                _current->flags.reply = c == '*';
                return false;
            } else
                return false;   //Noise
            break;
        case ParseID:
            _current->id = v.i;
            _current->flags.addressed = true;
            parser_state = ParseCommand;
            lexer_read_lexicon();
            goto restart_lexer;
        case ParseCommand:
            // successful match if we reached the terminal of the lexicon (c==0)
            // and we have a non-zero code (zero code would mean a non-match)
            if(lexicon_node->c == 0 && lexicon_node->code != TERMINAL_CODE) {
                // finished parsing command
                //
                _current->command = (command::ID)lexicon_node->code;

                // determine what kind of value we are getting if any
                if(c == '\r') {
                    parser_state = ParseTerminator;
                    goto restart_parser;
                } else if(c == '-' || IS_DIGIT(c)) {
                    parser_state = ParseIntValue;
                    // initialize the lexer to read an integer
                    lexer_read_int();
                    goto restart_lexer;
                }/* else if(!IS_DIGIT(c)) {
                    parser_state = ParseStringValue;
                } */
                  else if(c == '#') {
                    goto repeated_start_id;
                } else {
                    set_error(CorruptPacket);
                    return false;
                }
            } else {
                // unknown command
                set_error(UnknownCommand);
                return false;
            }
            break;
        case ParseIntValue:
            _current->args[_current->nargs++] = v.i;

            if(c == '\r') {
                goto terminate_packet;
            } else if (c == ',') {
                // ensure we arent reading more than supported number of arguments
                if (_current->nargs < MAX_ARGS - 1) {
                    // parse another value
                    lexer_read_int();
                } else {
                    set_error(TooManyArguments);
                }
                return false;
            } else if(c == '#' || c == '*') {
                goto repeated_start_id;
            } else if(IS_ALPHA(c)) {
                goto repeated_start_cmd;
            } else {
                // non alpha-numeric, return a parse error
                set_error(CorruptPacket);
                return false;
            }
        case ParseStringValue:
            // todo: support string packet values
            set_error(ExpectedIntValue);
            return false;
        case ParseTerminator:
            if (c == '#' || c == '*') {
            repeated_start_id:
                ////// Same code block as in repeated_start_cmd  (please refactor if possible) //////
                // encountered a repeated start with the same ID
                // add this command to the request stack

                // indicate part of a continuation (grouped set)
                _current->flags.parsed = true;
                _current->flags.reply = c == '*';
                _current->flags.continuation = true;
                ////// End block //////

                parser_state = ParseID;                 // and jump to reading the new ID
                lexer_read_unsigned();

                // clear request object on next pass
                parser_subroutine_state = ClearRequest;

                advance_request();
                return _count<=0;
            } else if (IS_ALPHA(c)) {
            repeated_start_cmd:
                // encountered a repeated start with the same ID
                // add this command to the request stack
                _current->flags.parsed = true;
                _current->flags.continuation = true;
                ////// End block //////

                parser_state = ParseCommand;            // and jump to reading the new packet command
                lexer_read_lexicon();

                // clear request object on next pass
                parser_subroutine_state = ClearLocalRequest;

                advance_request();

                // We must return the parsed request to the user in order to
                // free up the request object for this new command, but we
                // haven't processed the incoming character so we must save
                // it for the next call.
                unprocessed_char = c;
                return _count<=0;
            } else if(c == '\r') {
            terminate_packet:
                _current->flags.terminal = true;
                _current->flags.parsed = true;
                parser_state = Idle;
                advance_request();
                return _count<=0;
            } else {
                // encountered noise before the packet terminator
                // packet should be aborted
                set_error(CorruptPacket);
                return false;
            }
        case ClearRequest:
          _current->id = 0;
          // no break, intentional fall-through
        case ClearLocalRequest:
          // This is a state sub-routine that clears the request object
          // and then proceeds to the original parse state.
          parser_subroutine_state = Idle; // returns to original state
          _current->command = command::Unknown;
          _current->flags.parsed
              = _current->flags.addressed
              = _current->flags.continuation
              = _current->flags.terminal = false;
          _current->nargs = 0;
          memset(_current->args, 0, sizeof(_current->args));
          goto restart_parser;  // parse again with original state active
        case IgnorePacket:
            if (c == '#') {
                // encountered a repeated-start
                parser_state = Idle;
                goto packet_start;
            } else if (c == '\r') {
                parser_state = Idle;
                return false;
            } else
                return false;
        case ParsingError:
            if(c == '\r') {
                parser_state = Idle;
            } else if(c == '#' || c == '*') {
                // start of new packet without terminating old one
                parser_state = Idle;
                parser_subroutine_state = ClearRequest;
                goto packet_start;
            }
            return false;

        // todo: remove parser states that are not longer valid
        case Noise:
        case ParseSeperator:
        	set_error(ParserInternalError);
        	return false;

        default:
          assert(false);
    }

    // if compiler generates a "control reaches end of non-void function"
    // then check your new states, make sure each state either returns
    // or restarts parsing. The received character should be processed if
    // possible or saved in unprocessed_char.
}

} // ns: Lss
