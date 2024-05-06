//
// Created by guru on 10/6/21.
//
#include "LssSynthesize.h"

#include <catch2/catch.hpp>

using namespace lss;


/*
 * Response Parsing
 */

TEST_CASE("synth a simple command", "[synthesis]") {
  const char* validate = "#15QD960\r";
  Request req(15, command::QD, 960);
  Synthesize<32> synth;

  const char* result = synth(req);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a simple command with negative argument", "[synthesis]") {
  const char* validate = "#15QD-960\r";
  Request req(15, command::QD, -960);
  Synthesize<32> synth;

  const char* result = synth(req);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a local group command", "[synthesis]") {
  const char* validate = "#15QD960T300\r";
  Request reqs[] = {
    {15, command::QD, 960},
    {15, command::T, 300}
  };
  Synthesize<32> synth;

  const char* result = synth(reqs);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a multi-servo group command", "[synthesis]") {
  const char* validate = "#15QD960T300#5QD500T550\r";
  Request reqs[] = {
      {15, command::QD, 960},
      {15, command::T, 300},
      {5, command::QD, 500},
      {5, command::T, 550}
  };
  Synthesize<32> synth;

  const char* result = synth(reqs);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a multi-servo group command with 1 disabled", "[synthesis]") {
  const char* validate = "#15QD960#5QD500T550\r";
  Request reqs[] = {
      {15, command::QD, 960},
      {15, command::T, 300},
      {5, command::QD, 500},
      {5, command::T, 550}
  };
  reqs[1].enable = false;
  Synthesize<32> synth;

  const char* result = synth(reqs);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a multi-servo group command with first one disabled",
          "[synthesis]") {
  const char* validate = "#15T300#5QD500T550\r";
  Request reqs[] = {
      {15, command::QD, 960},
      {15, command::T, 300},
      {5, command::QD, 500},
      {5, command::T, 550}
  };
  reqs[0].enable = false;
  Synthesize<32> synth;

  const char* result = synth(reqs);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a multi-servo group command with last one disabled",
          "[synthesis]") {
  const char* validate = "#15QD960T300#5QD500\r";
  Request reqs[] = {
      {15, command::QD, 960},
      {15, command::T, 300},
      {5, command::QD, 500},
      {5, command::T, 550}
  };
  reqs[3].enable = false;
  Synthesize<32> synth;

  const char* result = synth(reqs);
  REQUIRE( strcmp(validate, result) == 0);
}

TEST_CASE("synth a multi-servo group command with first of second cmd disabled",
          "[synthesis]") {
  const char* validate = "#15QD960T300#5T550\r";
  Request reqs[] = {
      {15, command::QD, 960},
      {15, command::T, 300},
      {5, command::QD, 500},
      {5, command::T, 550}
  };
  reqs[2].enable = false;
  Synthesize<32> synth;

  const char* result = synth(reqs);
  REQUIRE( strcmp(validate, result) == 0);
}