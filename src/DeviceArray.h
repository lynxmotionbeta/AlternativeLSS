//
// Created by guru on 10/13/21.
//

#ifndef LSS_BUS_DEVICEARRAY_H
#define LSS_BUS_DEVICEARRAY_H

#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t


namespace Lss {

template<class T, uint8_t min=0, uint8_t max=253>
class DeviceArray
{
  public:
    using ArrayType = DeviceArray<T, min, max>;

    typedef struct {
        uint8_t present : 1;
    } Flags;

    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;  // or also value_type*
        using reference         = T&;  // or also value_type&

        Iterator(ArrayType& arr, uint8_t id) : _arr(arr), _id(id) {
            // advance to the first valid value
            while(!_arr.contains(_id) && _id <= max) {
                _id++;
            };
            if(_id > max) _id = 255;    // end
        }

        reference operator*() const { return _arr[_id]; }
        pointer operator->() { return &_arr[_id]; }

        Iterator& operator++() {
            do {
                _id++;
            }  while(!_arr.contains(_id) && _id <= max);
            if(_id > max) _id = 255;    // end
            return *this;
        }

        inline uint8_t id() const { return _id; }

        // Postfix increment
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const Iterator& a, const Iterator& b) { return a._id == b._id; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a._id != b._id; };

    private:
        ArrayType& _arr;
        uint8_t _id;
    };


  public:
    DeviceArray()
        //: flags({ .present = 0})
    {
        memset(flags, 0, sizeof(flags));
    }

    int scan(Bus& bus) {
        auto ids = bus.scan();
        int added = 0;
        for(auto id: ids) {
            if(id >= min && id <= max && !contains(id)) {
                add(id);
                added++;
            }
        }
        return added;
    }

    void add(uint8_t id) {
        assert(id >= min && id <= max);   // ensure id range
        id -= min;
        if(!flags[id].present) {
            // use placement operator to initialize memory
            flags[id].present = true;
            new (&devices[id]) T();
        }
    }

    void remove(uint8_t id) {
        assert(id >= min && id <= max);   // ensure id range
        id -= min;
        if(flags[id].present) {
            // placement delete
            devices[id].~T();
            ::operator delete(&devices[id]);
            flags[id].present = false;
        }
    }

    inline constexpr uint8_t size() const { return max - min + 1; }

    std::vector<uint8_t> id_list() const {
        std::vector<uint8_t> list;
        for(uint8_t n=0; n < size(); n++) {
            if(flags[n].present)
                list.insert(list.end(), min + n);
        }
        return list;
    }

    uint8_t count() const {
        uint8_t c = 0;
        for(uint8_t n=0; n < size(); n++) {
            if(flags[n].present)
                c++;
        }
        return c;
    }

    bool contains(uint8_t id) const {
        if(id < min || id > max)
            return false;
        id -= min;
        return flags[id].present;
    }

    Iterator begin() { return Iterator(*this, min); }
    Iterator end()   { return Iterator(*this, 255); }

    inline T& operator[](uint8_t id) {
        assert(id >= min && id <= max);   // ensure id range
        id -= min;
        return devices[id];
    }

    inline const T& operator[](uint8_t id) const {
        assert(id >= min && id <= max);   // ensure id range
        id -= min;
        return devices[id];
    }

  protected:
    Flags flags[max - min + 1];
    T devices[max - min + 1];

    static_assert(min < max, "min device ID must be less than max");
    static_assert(max < 254, "254 and 255 are reserved bus IDs and cannot be used (lower your DeviceArray max value to 253)");
};


}  // ns:Lss
#endif//LSS_BUS_DEVICEARRAY_H
