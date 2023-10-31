//
// Created by guru on 10/20/21.
//
#include <DeviceIndex.h>
#include <LssSynthesize.h>

#include <catch2/catch.hpp>

using namespace lss;


/*
 * Response Parsing
 */

TEST_CASE("create a DeviceIndex with two IDs", "[indexes]") {
  const char* validate = "#254SLOTCOUNT2#0SLOT0#5SLOT1\r";

  DeviceIndex index;
  index.append(0);
  index.append(5);

  REQUIRE(index.count() == 2);
  REQUIRE(index[0] == 0);
  REQUIRE(index[1] == 5);

  // all remaining elements are unassigned
  for(int i=2; i<254; i++) {
      REQUIRE(index[i] == 255);
  }
}

TEST_CASE("Inverted DeviceIndex with two IDs", "[indexes]") {
  const char* validate = "#254SLOTCOUNT2#0SLOT0#5SLOT1\r";

  DeviceIndex index;
  index.append(0);
  index.append(5);

  REQUIRE(index.count() == 2);

  DeviceIndex inverted = index.invert();
  for(int i=0; i<254; i++) {
    switch(i) {
    case 0:
      REQUIRE(inverted[i] == 0);
      break;
      case 5:
        REQUIRE(inverted[i] == 1);
        break;
        default:
          REQUIRE(inverted[i] == 255);
          break;
    }
  }
}

TEST_CASE("generating slots from a DeviceIndex with two IDs", "[indexes]") {
  const char* validate = "#254SLOTCOUNT2#0SLOT0#5SLOT1\r";

  DeviceIndex index;
  index.append(0);
  index.append(5);
  auto requests = generate_slot_config(index);

  REQUIRE( requests.size() == 3);
  REQUIRE( requests[0].id == BroadcastID);
  REQUIRE( requests[0].command == command::SLOTCOUNT);
  REQUIRE( requests[0].nargs == 1);
  REQUIRE( requests[0].args[0] == requests.size() - 1);

  REQUIRE( requests[1].id == 0);
  REQUIRE( requests[1].command == command::SLOT);
  REQUIRE( requests[1].nargs == 1);
  REQUIRE( requests[1].args[0] == 0);

  REQUIRE( requests[2].id == 5);
  REQUIRE( requests[2].command == command::SLOT);
  REQUIRE( requests[2].nargs == 1);
  REQUIRE( requests[2].args[0] == 1);
}


TEST_CASE("synthesizing slots from a DeviceIndex with two IDs", "[indexes]") {
  const char* validate = "#254SLOTCOUNT2#0SLOT0#5SLOT1\r";

  DeviceIndex index;
  index.append(0);
  index.append(5);
  auto requests = generate_slot_config(index);

  REQUIRE( requests.size() == 3);
  REQUIRE( requests[0].id == BroadcastID);
  REQUIRE( requests[0].command == command::SLOTCOUNT);
  REQUIRE( requests[0].nargs == 1);
  REQUIRE( requests[0].args[0] == requests.size() - 1);

  REQUIRE( requests[1].id == 0);
  REQUIRE( requests[1].command == command::SLOT);
  REQUIRE( requests[1].nargs == 1);
  REQUIRE( requests[1].args[0] == 0);

  REQUIRE( requests[2].id == 5);
  REQUIRE( requests[2].command == command::SLOT);
  REQUIRE( requests[2].nargs == 1);
  REQUIRE( requests[2].args[0] == 1);

  Synthesize<32> synth;
  const char* result = synth(requests);
  REQUIRE( strcmp(validate, result) == 0);
}
