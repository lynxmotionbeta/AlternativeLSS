//
// Created by guru on 10/17/21.
//

#include "DeviceIndex.h"
#include "LssBus.h"

namespace lss {

DeviceIndex::DeviceIndex()
: count_(0), is_inverted_(false)
{
  memset(servo_index_, 255, sizeof(servo_index_));
}

bool DeviceIndex::append(uint8_t id) {
  if(!contains(id)) {
    servo_index_[count_++] = id;
    return true;
  }
  return false;
}

int DeviceIndex::scan(Bus &bus) {
  auto ids = bus.scan();
  int added = 0;
  for (auto id : ids) {
    if (append(id))
      added++;
  }
  return added;
}

DeviceIndex DeviceIndex::invert() const {
  DeviceIndex inverted;
  inverted.is_inverted_ = !is_inverted_;
  for(auto i = begin(), _i = end(); i!=_i; i++) {
    inverted[servo_index_[i.key()]] = i.value();
    inverted.count_++;
  }
  return inverted;
}

} // ns:Lss
