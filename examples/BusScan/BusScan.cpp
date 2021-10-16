#include <LssBus.h>
#include <DeviceArray.h>

#include <cstdio>
#include <iostream>
#include <unistd.h>

using namespace Lss;

struct Servo {
  // This is an object you create, implement your
  // servo object to fit your application needs
};


int main() {
    // a channel represents a bus of servos and is attached to a Arduino Stream
    // (typically a HardwareSerial port)
    Bus bus;

    // put your setup code here, to run once:
    bus.open("/dev/ttyUSB0", 921600);

    /*
     * Method 1
     *
     * Bus Scan and fetch list of IDs present
     */

    printf("scanning for servos using bus directly:");
    auto ids = bus.scan();    // returns a vector of 8bit ints
    for(auto id: ids) {
      printf(" %d", id);
    }
    printf("\n");


    /*
     * Method 2
     *
     * Allocate a DeviceArray and perform a bus scan to create objects
     * of a custom user type.
     */

    // allocate an array of servo objects,
    // here we limit the array to only include IDs 0 to 75, any outside
    // this range will be ignored.
    DeviceArray<Servo, 0, 75> servos;

    printf("scanning for servos using DeviceArray:");
    // you can repeat the scan a few times to make sure we get everyone
    servos.scan(bus);
    usleep(50000);
    servos.scan(bus);
    usleep(50000);
    servos.scan(bus);

    // build an index of Lss Servo ID to our Servo array
    for(auto s = servos.begin(), _s = servos.end(); s != _s; s++) {
        printf(" %d", s.id());
    }
    printf("\n");

    // now proceed to ReadStateLoop example to see how you can
    // fill your Servo object with properties from servo state.
    return 0;
}
