//
// Created by guru on 10/16/21.
//
#include "LssChannelDriver.h"

namespace lss {
namespace platform {

  const char* lss_bus_driver_error_str(ChannelDriverError err) {
    switch(err) {
    case DriverSuccess: return "success";
    case DriverOpenFailed: return "open failed";
    case DriverAlreadyInitialized: return "already initialized";
    case DriverNotFound: return "not found";
    case DriverAccessDenied: return "access denied";
    case DriverInvalidParameter: return "invalid parameter";
    default:
      return "unknown";
    }
  }

} // ns:Platform
} // ns:Lss
