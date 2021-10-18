//
// Created by guru on 10/4/21.
//

#include "LssBus.h"
#include <unistd.h>

namespace lss {


bool Bus::async_scan() {
    Request req(BroadcastID, command::QID, scan_slot_size_usec);
    channel.write(synthesize(req));
    return true;
}

std::vector<uint8_t> Bus::async_scan_complete() {
    char buffer[256];
    int bytes_read;
    std::vector<uint8_t> ids;

    // todo: perhaps record the scan start time, check that the required
    //  amount of time has passed and either return fail or block

    // parse a single request at a time
    Request reply;
    parser.begin(reply);

    while((bytes_read = channel.read(buffer, sizeof(buffer))) >0) {
        const char* p = buffer;
        while(bytes_read-- > 0) {
            if (parser.parse(*p++) && reply.flags.parsed) {
                // got a reply
                ids.insert(ids.end(), reply.id);
            }
        }
    }
    return ids;
}


std::vector<uint8_t> Bus::scan() {
    async_scan();
    usleep(scan_slot_size_usec * 255);
    return async_scan_complete();
}

} //ns:Lss
