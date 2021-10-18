#include <LssBus.h>
#include <DeviceArray.h>

#include <cstdio>
#include <iostream>
#include <unistd.h>

using namespace lss;

/// Repeat the update 4 times a second
//#define UPDATE_DELAY			12

// timing budgets for our control loop
// these are measured using Saleae probe in microseconds
#define REPLY_DELAY              3000
#define PER_SERVO_DELAY          500


typedef struct {
    int position;
    int amps;
    int speed;
} Servo;

int main() {
    // servo index will quickly tell us where in the Servo array a certain ID exists
    DeviceArray<Servo, 0, 75> servos;

    // a channel represents a bus of servos and is attached to a Arduino Stream
    // (typically a HardwareSerial port)
    Bus bus;

    // put your setup code here, to run once:
    bus.open("/dev/ttyUSB0", 921600);

    printf("scanning for servos:");
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

    // configure the servos to reply as a group
    // So only one \r will appear per group request
    //bus.write(Request(BroadcastID, Command::PROTOCOL, 1, 0));

    // continuously read state information of up to 20 servos
    Request state_req[60];
    size_t state_n = 0;
    for(auto s = servos.begin(), _s = servos.end(); s!=_s; s++) {
        state_req[state_n++] = Request(s.id(), command::QD);
        state_req[state_n++] = Request(s.id(), command::QC);
        state_req[state_n++] = Request(s.id(), command::QS);
    }

    // estimate what our loop frequency can comfortably be
    // todo: add this as a feature of the bus I think
    int min_loop_frequency = REPLY_DELAY + PER_SERVO_DELAY * servos.count();
    printf("expected transaction time of %duSec\n", min_loop_frequency);
    min_loop_frequency = (min_loop_frequency / 250 + 1) *
                         250;
    printf("choosing loop frequency of %duSec (%4.2f Hz)\n", min_loop_frequency,  (double)1000000 / min_loop_frequency);

    size_t n;
    size_t N = 1000000;
    while(N-- > 0) {
        // write a request that will fill our state objects
        bus.request(state_req, state_n);

        usleep(min_loop_frequency);

        // wait some time before expecting an answer
        if(0 < (n = bus.update(state_req, state_n))) {
            Request* preq = state_req;
            if(n < state_n) {
                printf("partial\n");
            }
            n = state_n;
            while(n-- > 0) {
                // If one or more servos didn't reply then some requests
                // in our state_reply may not have been parsed, we'll just
                // catch'em the next round.
                if(preq->flags.parsed) {
                    auto & servo = servos[preq->id];
                    switch (preq->command) {
                        case command::QD: servo.position = preq->args[0]; break;
                        case command::QC: servo.amps = preq->args[0]; break;
                        case command::QS: servo.speed = preq->args[0]; break;
                        default: break;// nothing
                    }
                }
                preq++;
            }
        }
    }

    return 0;
}
