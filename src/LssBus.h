//
// Created by guru on 10/4/21.
//

#ifndef LSS_BUS_LSSBUS_H
#define LSS_BUS_LSSBUS_H

#include "LssParser.h"
#include "LssChannel.h"
#include "LssSynthesize.h"

#include <vector>

#include <cstdio>       // todo: get rid of all printf debugging statements
#include <unistd.h>

namespace Lss {

class Bus {
public:
  inline Bus()
  : phead(nullptr), bytes_left(0)
  {}

#if defined(ARDUINO)
  // open port using Arduino stream interface
  inline Platform::ChannelDriverError begin(Stream & dev, int baudrate) {
      return channel.begin(dev, baudrate);
  }
#else
  // open port using standard linux /dev name or ftdi:<vendor>:<product>:<A,B,C,D>
  inline Platform::ChannelDriverError open(const char * devname, int baudrate) {
      return channel.open(devname, baudrate);
  }
#endif

    inline void close() {
        return channel.close();
    }

    bool async_scan();
    std::vector<uint8_t> async_scan_complete();
    std::vector<uint8_t> scan();

    template<size_t N>
    int write(Request (&arr)[N]) {
        return write(arr, arr + N);
    }

    template<size_t N>
    int write(Request (&arr)[N], size_t limitN) {
        limitN = std::min(N, limitN);
        return write(arr, arr + limitN);
    }

    int write(Request& request) {
        channel.write(synthesize(request));
        return 1;
    }

    int write(Request&& request) {
        channel.write(synthesize(request));
        return 1;
    }

    int write(Request* begin, Request* end) {
        channel.write(synthesize(begin, end));
        return end - begin;
    }

    template<size_t N>
    int request(Request (&arr)[N], size_t limitN) {
        limitN = std::min(N, limitN);
        return request(arr, arr + limitN);
    }

    int request(Request* begin, Request* end) {
        auto test = synthesize.request(begin, end);
        channel.write(test);
        return end - begin;
    }


    // todo: add read() function for receiving packets without synchronizing into an existing array

    template<size_t N>
    int update(Request (&arr)[N]) {
        return update(arr, arr + N);
    }

    template<size_t N>
    int update(Request (&arr)[N], size_t limitN) {
        limitN = std::min(N, limitN);
        return update(arr, arr + limitN);
    }

    // read until a \r is encountered
    // which may be a single request or a group reply
    int update(Request* begin, Request* end)
    {
        int parsed = 0;
        Request req;
        parser.begin(req);

        // todo: would be nice if this read just parsed until end of data
        //    and then returned what? a count of parsed requests? how does
        //    the user know all his requests have been read?
        int timeout = read_timeout;
        while (--timeout) {  // todo: convert this to a timeout
            while (data_available()) {
                if(parse()) {
                    // got a reply
                    while(req.id != begin->id || req.command != begin->command) {
                        // advance until we match
                        begin->flags.parsed = false;
                        begin->flags.reply = false;
                        begin->nargs = 0;

                        begin++;
                        if(begin >= end) {
                            // cannot sync this packet with input array
                            // todo: we kind of need to put this packet back on the stack? - how?
                            printf("out-of-sync\n");
                            return parsed;
                        }
                    }

                    *begin++ = req;
                    parsed++;
                    if(begin == end)
                        return parsed;
                }
            }
            usleep(100);
        }
        return parsed;
    }

    void read_async(Request& reply) {
        parser.begin(reply);
    }


    int read_async(Request* begin, Request* end, bool sync) {
    // block read until all packets are full
    parser.begin(begin, end);
    return 0;
    }

protected:
    Channel channel;
    Parser parser;
    Synthesize<1024> synthesize;

    // our read buffer
    char buffer[256];
    char* phead;
    int bytes_left;

    static const int read_timeout = 50;         // in tenths of a millisecond
    static const int scan_slot_size_usec = 150;

    inline bool data_available() {
        return bytes_left>0 || channel.data_available();
    }

    bool parse() {
        if(bytes_left == 0) {
            // read another chunk
            if(channel.data_available()) {
                bytes_left = channel.read(buffer, sizeof(buffer));
                phead = buffer;
                if (bytes_left <= 0)
                    return false;// no data left
            } else
                return false;
        }
        bytes_left--;
        return parser.parse(*phead++);
    }
};

} // ns:Lss
#endif // LSS_BUS_LSSBUS_H