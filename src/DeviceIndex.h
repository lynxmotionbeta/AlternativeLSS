//
// Created by guru on 10/17/21.
//

#ifndef LYNXMOTION_LSS_DEVICEINDEX_H
#define LYNXMOTION_LSS_DEVICEINDEX_H

#include <cstdint>
#include <cstring>
#include <vector>

namespace lss {

class Bus;

class DeviceIndex
{
public:
  template<class AT>
  struct ForwardIterator {
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = uint8_t;
      using pointer           = value_type*;  // or also value_type*
      using reference         = value_type&;  // or also value_type&

      ForwardIterator(AT& arr, uint8_t idx) : _arr(arr), _idx(idx) {
        // advance to the first valid value
        while(_arr[_idx] == 255 && _idx < 255) {
          _idx++;
        };
      }

      reference operator*() const { return _arr[_idx]; }
      pointer operator->() { return &_arr[_idx]; }

      ForwardIterator & operator++() {
        do {
          _idx++;
        }  while(_arr[_idx]==255 && _idx < 255);
        return *this;
      }

      inline uint8_t key() const { return _idx; }
      inline uint8_t value() const { return _arr[_idx]; }

      // Postfix increment
      ForwardIterator operator++(int) {
        ForwardIterator tmp = *this; ++(*this); return tmp;
      }

      friend bool operator== (const ForwardIterator & a, const ForwardIterator & b) { return a._idx == b._idx; };
      friend bool operator!= (const ForwardIterator & a, const ForwardIterator & b) { return a._idx != b._idx; };

      private:
        AT& _arr;
        uint8_t _idx;
  };

  using Iterator = ForwardIterator<DeviceIndex>;
  using ConstIterator = ForwardIterator<const DeviceIndex>;

  inline Iterator begin() { return Iterator(*this, 0); }
  inline Iterator end()   { return Iterator(*this, 255); }
  inline ConstIterator begin() const { return ConstIterator(*this, 0); }
  inline ConstIterator end() const   { return ConstIterator(*this, 255); }

  DeviceIndex();

  int scan(Bus& bus);

  void clear();

  bool append(uint8_t id);

  inline bool contains(uint8_t id) const {
    return servo_index_[id] != 255;
  }

  inline uint8_t operator[](uint8_t idx) const {
    return servo_index_[idx];
  }

  inline uint8_t& operator[](uint8_t idx) {
    return servo_index_[idx];
  }

  DeviceIndex invert() const;

protected:
  uint8_t servo_index_[255];
  uint8_t count_;
  bool is_inverted_;
};


}  // ns:Lss

#endif // LYNXMOTION_LSS_DEVICEINDEX_H
