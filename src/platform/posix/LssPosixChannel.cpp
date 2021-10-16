#include "LssPosixChannel.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
//#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <pthread.h>

#include <functional>
#include <poll.h>
#include <asm/ioctls.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */


#if 0
#define IFLOG(x) x
#else
#define IFLOG(x)
#endif

namespace Lss {
namespace Platform {

PosixChannel::PosixChannel()
: devname(nullptr), baudrate(115200), fd(0), restore_tio(false), restore_serial_flags(0)
{
}

PosixChannel::~PosixChannel() {
    close();
}

ChannelDriverError PosixChannel::open(const char* _devname, int _baudrate)
{
    close();

    // copy the arguments
    if(devname) ::free((void*)devname);
    devname = strdup(_devname);
    baudrate = _baudrate;

    // our port settings, and saved settings to restore when closing
    struct termios oldtio, newtio;
    int old_serial_flags;
    bool restore_tio = false;

    /* open the device to be non-blocking (read will return immediately) */
    fd = ::open(devname, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if (fd < 0) {
        perror(devname);
        switch(errno) {
            case EACCES: return DriverAccessDenied;
            case EBADF:
            case ENOENT:
                return DriverNotFound;
            default:
                return DriverOpenFailed;
        }
    }

    struct serial_struct serial;
    ioctl(fd, TIOCGSERIAL, &serial);
    restore_serial_flags = serial.flags;

    configure_port();

    return DriverSuccess;
}

void PosixChannel::close()
{
    if(fd > 0) {
        restore_port_settings();
        ::close(fd);
        fd = 0;
    }
}

bool PosixChannel::low_latency(bool enable)
{
    /* Make the file descriptor asynchronous (the manual page says only
       O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
    //fcntl(fd, F_SETFL, FASYNC);
    struct serial_struct serial;
    ioctl(fd, TIOCGSERIAL, &serial);
    restore_serial_flags = serial.flags;
    serial.flags |= ASYNC_LOW_LATENCY;
    return 0 == ioctl(fd, TIOCSSERIAL, &serial);
}

bool PosixChannel::configure_port()
{
    struct termios newtio;
    tcgetattr(fd,&oldtio); /* save current port settings */
    newtio.c_cflag &= ~CBAUD; // no common baud rate
    newtio.c_cflag = CBAUDEX | CS8 | CLOCAL | CREAD; // | CRTSCTS  // use extended baud rates
    newtio.c_cflag &= ~(PARENB | PARODD); // No parity
    newtio.c_cflag &= ~CRTSCTS; // No hardware handshake
    newtio.c_cflag &= ~CSTOPB; // 1 stopbit

    newtio.c_iflag = IGNBRK;
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY); // No software handshake
    newtio.c_iflag = IGNPAR;

    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */

    // good discussion on setting extended baudrates at:
    // https://stackoverflow.com/questions/12646324/how-can-i-set-a-custom-baud-rate-on-linux
    cfsetspeed(&newtio, baudrate);  // select baudrate

    newtio.c_cc[VMIN]=1;
    newtio.c_cc[VTIME]=0;
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW, &newtio);
    restore_tio = true; // remember to restore old settings

    return true;
}

void PosixChannel::restore_port_settings() {
    if(fd > 0) {
        // restore old port settings
        if (restore_tio)
            tcsetattr(fd, TCSANOW, &oldtio);

        struct serial_struct serial;
        ioctl(fd, TIOCGSERIAL, &serial);
        serial.flags = restore_serial_flags;
        ioctl(fd, TIOCSSERIAL, &serial);
    }
}

int PosixChannel::data_available()
{
    int bytes_available;
    return (0 == ioctl(fd, FIONREAD, &bytes_available))
        ? bytes_available
        : -1;
}

int PosixChannel::write(const char* text, int text_len)
{
    if (text_len < 0)
        text_len = strlen(text);
    return (fd > 0)
        ? ::write(fd, text, text_len)
        : 0;
}

int PosixChannel::read(char* dest, int max_length)
{
    return (fd > 0)
        ? ::read(fd, dest, max_length)
        : 0;
}


} //ns:Platform
} //ns:Lss



