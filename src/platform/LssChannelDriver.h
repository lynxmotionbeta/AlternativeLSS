//
// Created by guru on 12/13/19.
//

#pragma once

#include <cstdint>

namespace Lss {
namespace Platform {

typedef enum ChannelState {
    ChannelStopped,
    ChannelError,
    ChannelStarting,
    ChannelStopping,
    ChannelIdle,
    ChannelProcessing
} ChannelState;

typedef enum {
    DriverSuccess,
    DriverUnknownError,
    DriverOpenFailed,
    DriverAlreadyInitialized,
    DriverNotFound,
    DriverAccessDenied,
    DriverInvalidParameter
} ChannelDriverError;


typedef enum {
    OpenSignal,
    UpdateSignal,
    DataSignal,
    TransmitSignal,
    TransactionSignal
} ChannelDriverSignal;

class LssChannel;

class LssChannelDriver
{
public:
    virtual ~LssChannelDriver() {};

    virtual ChannelDriverError open(const char* devname, int baudrate)=0;

    virtual void close()=0;

    virtual int data_available()=0;

    virtual int write(const char* text, int text_len = -1)=0;

    virtual int read(char* dest, int max_length)=0;
};

} //ns:Platform
} //ns:Lss
