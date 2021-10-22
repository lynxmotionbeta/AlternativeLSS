//
// Created by guru on 10/20/21.
//
#include "LssSynthesize.h"
#include "DeviceIndex.h"

namespace lss {

std::vector<Request> generate_slot_config(DeviceIndex& index) {
  std::vector<Request> reqs;
  reqs.emplace_back(BroadcastID, command::SLOTCOUNT, 0);
  uint8_t n = 0;
  for(auto i = index.begin(), _i = index.end(); i!=_i; i++) {
    reqs.emplace_back(i.value(), command::SLOT, i.key());
    n++;
  }

  // update the slot count
  reqs[0].args[0] = n;

  return reqs;
}

}