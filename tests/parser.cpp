#include "LssParser.h"

#include <catch2/catch.hpp>

using namespace lss;


/*
 * Response Parsing
 */

TEST_CASE("parse a simple response", "[parser]") {
  const char* data = "*15QD960\r";
  Parser p;
  Request req;

  p.begin(req);
  while(*data) {
    if(p.parse(*data++)) {
      REQUIRE(req.id == 15);
      REQUIRE(req.nargs == 1);
      REQUIRE(req.args[0] == 960);
      REQUIRE(req.command == command::QD);
      REQUIRE(req.flags.addressed);
      REQUIRE(req.flags.reply);
      REQUIRE(req.flags.parsed);
      REQUIRE_FALSE(req.flags.continuation);
      REQUIRE(req.flags.terminal);
    }
  }
}

TEST_CASE("parse a local group response", "[parser]") {
  const char* data = "*15QD960T400\r";
  Parser p;
  Request req;
  int seq = 0;

  p.begin(req);
  while(*data) {
    if(p.parse(*data++)) {
      switch(seq) {
        case 0:
          REQUIRE(req.id == 15);
          REQUIRE(req.nargs == 1);
          REQUIRE(req.args[0] == 960);
          REQUIRE(req.command == command::QD);
          REQUIRE(req.flags.addressed);
          REQUIRE(req.flags.reply);
          REQUIRE(req.flags.parsed);
          REQUIRE(req.flags.continuation);
          REQUIRE_FALSE(req.flags.terminal);
          break;
        case 1:
          REQUIRE(req.id == 15);
          REQUIRE(req.nargs == 1);
          REQUIRE(req.args[0] == 400);
          REQUIRE(req.command == command::T);
          REQUIRE_FALSE(req.flags.addressed);
          REQUIRE(req.flags.reply);
          REQUIRE(req.flags.parsed);
          REQUIRE_FALSE(req.flags.continuation);
          REQUIRE(req.flags.terminal);
          break;
      }
      seq++;
    }
  }
  REQUIRE(seq == 2);
}

TEST_CASE("parse a group response of 2 servos", "[parser]") {
  const char* data = "*15QD960T400*5QD500T750\r";
  Parser p;
  Request req;
  int seq = 0;

  p.begin(req);
  while(*data) {
    if(p.parse(*data++)) {
      switch(seq) {
        case 0:
          REQUIRE(req.id == 15);
          REQUIRE(req.nargs == 1);
          REQUIRE(req.args[0] == 960);
          REQUIRE(req.command == command::QD);
          REQUIRE(req.flags.addressed);
          REQUIRE(req.flags.reply);
          REQUIRE(req.flags.parsed);
          REQUIRE(req.flags.continuation);
          REQUIRE_FALSE(req.flags.terminal);
          break;
        case 1:
          REQUIRE(req.id == 15);
          REQUIRE(req.nargs == 1);
          REQUIRE(req.args[0] == 400);
          REQUIRE(req.command == command::T);
          REQUIRE_FALSE(req.flags.addressed);
          REQUIRE(req.flags.reply);
          REQUIRE(req.flags.parsed);
          REQUIRE(req.flags.continuation);
          REQUIRE_FALSE(req.flags.terminal);
          break;
        case 2:
          REQUIRE(req.id == 5);
          REQUIRE(req.nargs == 1);
          REQUIRE(req.args[0] == 500);
          REQUIRE(req.command == command::QD);
          REQUIRE(req.flags.addressed);
          REQUIRE(req.flags.reply);
          REQUIRE(req.flags.parsed);
          REQUIRE(req.flags.continuation);
          REQUIRE_FALSE(req.flags.terminal);
          break;
        case 3:
          REQUIRE(req.id == 5);
          REQUIRE(req.nargs == 1);
          REQUIRE(req.args[0] == 750);
          REQUIRE(req.command == command::T);
          REQUIRE_FALSE(req.flags.addressed);
          REQUIRE(req.flags.reply);
          REQUIRE(req.flags.parsed);
          REQUIRE_FALSE(req.flags.continuation);
          REQUIRE(req.flags.terminal);
          break;
      }
      seq++;
    }
  }
  REQUIRE(seq == 4);
}

TEST_CASE("parse a group response of 2 servos seperated by CR", "[parser]") {
  const char* data = "*15QD960T400\r*5QD500T750\r";
  Parser p;
  Request req;
  int seq = 0;

  p.begin(req);
  while(*data) {
    if(p.parse(*data++)) {
      switch(seq) {
      case 0:
        REQUIRE(req.id == 15);
        REQUIRE(req.nargs == 1);
        REQUIRE(req.args[0] == 960);
        REQUIRE(req.command == command::QD);
        REQUIRE(req.flags.addressed);
        REQUIRE(req.flags.reply);
        REQUIRE(req.flags.parsed);
        REQUIRE(req.flags.continuation);
        REQUIRE_FALSE(req.flags.terminal);
        break;
      case 1:
        REQUIRE(req.id == 15);
        REQUIRE(req.nargs == 1);
        REQUIRE(req.args[0] == 400);
        REQUIRE(req.command == command::T);
        REQUIRE_FALSE(req.flags.addressed);
        REQUIRE(req.flags.reply);
        REQUIRE(req.flags.parsed);
        REQUIRE_FALSE(req.flags.continuation);
        REQUIRE(req.flags.terminal);
        break;
      case 2:
        REQUIRE(req.id == 5);
        REQUIRE(req.nargs == 1);
        REQUIRE(req.args[0] == 500);
        REQUIRE(req.command == command::QD);
        REQUIRE(req.flags.addressed);
        REQUIRE(req.flags.reply);
        REQUIRE(req.flags.parsed);
        REQUIRE(req.flags.continuation);
        REQUIRE_FALSE(req.flags.terminal);
        break;
      case 3:
        REQUIRE(req.id == 5);
        REQUIRE(req.nargs == 1);
        REQUIRE(req.args[0] == 750);
        REQUIRE(req.command == command::T);
        REQUIRE_FALSE(req.flags.addressed);
        REQUIRE(req.flags.reply);
        REQUIRE(req.flags.parsed);
        REQUIRE_FALSE(req.flags.continuation);
        REQUIRE(req.flags.terminal);
        break;
      }
      seq++;
    }
  }
  REQUIRE(seq == 4);
}

TEST_CASE("parse a group response of 2 servos into an array", "[parser]") {
  const char* data = "*15QD960T400*5QD500T750\r";
  Parser p;
  Request req[4];
  int seq = 0;

  p.begin(req, 4);
  while(*data) {
    if(p.parse(*data++)) {
      // simply record that we parsed a new packet
      seq++;
    }
  }

  // sequence should be 1 since we requested all 4 to be read together
  REQUIRE(seq == 1);

  REQUIRE(req[0].id == 15);
  REQUIRE(req[0].nargs == 1);
  REQUIRE(req[0].args[0] == 960);
  REQUIRE(req[0].command == command::QD);
  REQUIRE(req[0].flags.addressed);
  REQUIRE(req[0].flags.reply);
  REQUIRE(req[0].flags.parsed);
  REQUIRE(req[0].flags.continuation);
  REQUIRE_FALSE(req[0].flags.terminal);

  REQUIRE(req[1].id == 15);
  REQUIRE(req[1].nargs == 1);
  REQUIRE(req[1].args[0] == 400);
  REQUIRE(req[1].command == command::T);
  REQUIRE_FALSE(req[1].flags.addressed);
  REQUIRE(req[1].flags.reply);
  REQUIRE(req[1].flags.parsed);
  REQUIRE(req[1].flags.continuation);
  REQUIRE_FALSE(req[1].flags.terminal);

  REQUIRE(req[2].id == 5);
  REQUIRE(req[2].nargs == 1);
  REQUIRE(req[2].args[0] == 500);
  REQUIRE(req[2].command == command::QD);
  REQUIRE(req[2].flags.addressed);
  REQUIRE(req[2].flags.reply);
  REQUIRE(req[2].flags.parsed);
  REQUIRE(req[2].flags.continuation);
  REQUIRE_FALSE(req[2].flags.terminal);

  REQUIRE(req[3].id == 5);
  REQUIRE(req[3].nargs == 1);
  REQUIRE(req[3].args[0] == 750);
  REQUIRE(req[3].command == command::T);
  REQUIRE_FALSE(req[3].flags.addressed);
  REQUIRE(req[3].flags.reply);
  REQUIRE(req[3].flags.parsed);
  REQUIRE_FALSE(req[3].flags.continuation);
  REQUIRE(req[3].flags.terminal);
}
