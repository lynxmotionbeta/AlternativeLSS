#pragma once

#include "../LssChannelDriver.h"

#include <termios.h>

namespace lss {
    namespace platform {


class PosixChannel : public LssChannelDriver {
public:
    explicit PosixChannel();
    ~PosixChannel() override;

    ChannelDriverError open(const char* devname, int baudrate)  override;

    void close() override;

    int data_available() override;

    int write(const char* text, int text_len) override;

    int read(char* dest, int max_length) override;

    // configure the port into low-latency mode
    bool low_latency(bool enable);

protected:
    bool configure_port();

    void restore_port_settings();

private:
    const char* devname;
    int baudrate;

    int fd;

    struct termios oldtio;
    bool restore_tio;

    bool restore_serial_flags;
};

    }   // ns:Platform
} // ns:Lss

