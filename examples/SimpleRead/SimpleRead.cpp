#include <LssBus.h>
#include <DeviceArray.h>

#include <cstdio>
#include <iostream>
#include <unistd.h>

using namespace Lss;


int main() {
  // a channel represents a bus of servos and is attached to a Arduino Stream
  // (typically a HardwareSerial port)
  Bus bus;

  // put your setup code here, to run once:
  bus.open("/dev/ttyUSB0", 921600);

  Request position_req(5, Command::QD);

  size_t N = 10;
  while(N-- > 0) {
    // write a request that will fill our state objects
    bus.request(position_req);

    // wait some time before expecting an answer
    if(0 != bus.update(position_req)) {
      if (position_req.nargs > 0 && position_req.command == Command::QD) {
        printf("  %d\n", position_req.args[0]);
      }
    }

    // repeat every 1 second
    sleep(1);
  }

  return 0;
}
