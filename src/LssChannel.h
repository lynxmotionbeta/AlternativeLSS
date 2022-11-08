#pragma once

#include <cassert>

#if defined(LSS_STATS)
#include "analytics/aggregate.h"
#endif

#include <list>
#include <utility>
#include <cstddef>

class LynxServo;

#if defined(ARDUINO)
#include "LssArduinoChannel.h"
#else
#include "platform/posix/LssPosixChannel.h"
#endif

#if defined(HAS_LIBFTDI)
#include "platform/libftdi/LssFtdiChannel.h"
#endif

namespace lss {

class Channel
{
  public:
    Channel();
    virtual ~Channel();

#if defined(ARDUINO)
    // open port using Arduino stream interface
    ChannelDriverError begin(Stream & dev, int baudrate);
#else
    // open port using standard linux /dev name or ftdi:<vendor>:<product>:<A,B,C,D>
    platform::ChannelDriverError open(const char * devname, int baudrate);
#endif

    inline void close()
    {
        if (_driver) {
            delete _driver;
            _driver = nullptr;
        }
    }

    inline bool ready() const
    {
        return _driver != nullptr;
    }

    inline int data_available()
    {
        assert(_driver);
        return _driver->data_available();
    }

    inline int write(const char * text, int text_len = -1)
    {
        if(text) {
            assert(_driver);
            return _driver->write(text, text_len);
        } else return 0;
    }

    int read(char * dest, size_t max_length)
    {
        assert(_driver);
        return _driver->read(dest, max_length);
    }

    // todo: probably want a function to get access to handles that can be
    //  used in OS poll() for use in an Executer which can poll multiple buses.

    const platform::LssChannelDriver & driver() const { return *_driver; }
    platform::LssChannelDriver & driver() { return *_driver; }

  protected:
    platform::LssChannelDriver * _driver;
};

} // ns: Lss
