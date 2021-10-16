
#include "LssChannel.h"

#include <stdio.h>
#include <string.h>

namespace Lss
{

Channel::Channel()
    : _driver(nullptr)
{
}

Channel::~Channel()
{
    close();
}

#if defined(ARDUINO)
ChannelDriverError LssChannel::begin(Stream & dev, int baudrate)
{
    // open the driver channel
    _driver = new LssArduinoChannel(this);
    ChannelDriverError rv = (ChannelDriverError) (_driver->signal(OpenSignal, baudrate, &dev));
    if (rv != DriverSuccess) {
        delete _driver;
        Serial.println("failed to open Arduino serial channel");
    }
    return rv;
}
#else
Platform::ChannelDriverError Channel::open(const char * devname, int baudrate)
{
    if (strncasecmp("ftdi:", devname, 5) == 0) {
#if defined(HAS_LIBFTDI)
        _driver = new LssFtdiChannel(this);
        devname += 5;
#else
        printf("ftdi support not enabled\n");
#endif
    } else {
        _driver = new Platform::PosixChannel();
    }

    if (_driver) {
        // open the driver channel
        Platform::ChannelDriverError rv = _driver->open(devname, baudrate);
        if (rv != Platform::DriverSuccess)
            delete _driver;

        return rv;
    } else
        return Platform::DriverOpenFailed;
}
#endif

} // ns:Lss
