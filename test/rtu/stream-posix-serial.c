
#include "stream-posix-serial.h"
#include <emodbus/base/add/container_of.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 16

static int input_stream_on_write(struct input_stream_t* _this,
                                 const void* _data,
                                 unsigned int _size) {
    int r;
    struct stream_posix_serial_t* psp = container_of(_this, struct stream_posix_serial_t, input_stream);
    const int res = write(psp->f, _data, _size);
//    printf("%s sz(%d), f(%d)\n", __PRETTY_FUNCTION__, _size, psp->f);
    fsync(psp->f);
    if((r = event_add(psp->write_event, NULL)))
        fprintf(stderr, "Error with event_add() call: %m (%d)\n", r);
    return res;
}

static int output_stream_on_read(struct output_stream_t* _this,
                          void* _data,
                          unsigned int _size) {
    struct stream_posix_serial_t* psp = container_of(_this, struct stream_posix_serial_t, output_stream);
    return read(psp->f, _data, _size);
}

static void fd_event(evutil_socket_t fd, short what, void *arg) {
    struct stream_posix_serial_t* _this = (struct stream_posix_serial_t*)arg;

    char buffer[BUFFER_SIZE];

    int t;

//    printf("%s(%d, 0x%02X)\n", __FUNCTION__, fd, what);

    if(what & EV_READ) {
        t = read(_this->f, buffer, BUFFER_SIZE);
        if(t > 0)
            stream_write(&_this->output_stream, buffer, t);
    }

    if(what & EV_WRITE) {
        t = stream_read(&_this->input_stream, buffer, BUFFER_SIZE);
        if(t > 0) {
            write(_this->f, buffer, t);
            fsync(_this->f);
        }
        else if(!t)     // no more data available, stop the transmitting.
            event_del(_this->write_event);
    }
}

int stream_posix_serial_open(struct stream_posix_serial_t* _psp,
                           struct event_base *_ev_base,
                           const char* _dev_name,
                           unsigned int _baudrate) {

    int r = 0;

    struct termios options;

    _psp->input_stream.on_write = input_stream_on_write;
    _psp->output_stream.on_read = output_stream_on_read;

    do {
        _psp->f = open(_dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if(_psp->f < 0) {
            fprintf(stderr, "Error while open \"%s\" serial port: %m\n", _dev_name);
            r = -1;
            break;
        }

        tcgetattr(_psp->f, &options);

        // ****** c_cflag field setup (Управляющие опции)

        // Поле c_cflag содержит две опции, которые всегда
        // должны быть разрешены: CLOCAL и CREAD.
        options.c_cflag |= (CLOCAL | CREAD);
        //options.c_cflag &= ~(ICANON | IEXTEN | FLUSHO | PENDIN | TOSTOP);
        //options.c_cflag |= (NOFLSH);
        options.c_cflag &= ~PARENB;	// Disable parity bit
        options.c_cflag &= ~CSTOPB;	// 1 stop bit
        options.c_cflag &= ~CSIZE;	// Маскирование битов размера символов (CS5,CS8...)
        options.c_cflag |= CS8;		// 8 data bits

    // Disable hardware flow control
#ifdef CRTSCTS
        options.c_cflag &= ~CRTSCTS;
#elif defined CNEW_RTSCTS
        options.c_cflag &= ~CNEW_RTSCTS;
#endif

        // ****** c_lflag field setup (Локальные опции)
        // Выбор неканонического (Raw) ввода:
        options.c_lflag &= ~( // Next flags will disabled:
            ICANON | 	// Разрешить канонический ввод (иначе неканонический)
            ECHO | 		// Разрешить эхо вводимых символов
            ECHOE | 	// Символ эхо стирания как BS-SP-BS
            ISIG);		// Разрешить SIGINTR, SIGSUSP, SIGDSUSP, и SIGQUIT сигналы

        // ****** c_iflag field setup (Опции ввода)
        options.c_iflag &= ~( // Next flags will disabled:
            IXON | 		// Enable software flow control (outgoing)
            IXOFF | 	// Enable software flow control (incoming)
            IXANY | 	// Allow any character to start flow again
            INPCK | 	// Enable parity check
        //	IGNPAR | 	// Ignore parity errors
            PARMRK | 	// Mark parity errors
            ISTRIP | 	// Strip parity bits
        //	IGNBRK | 	// Ignore break condition
            IMAXBEL | 	// Echo BEL on input line too long
            INLCR | 	// Map CR to NL
            IGNCR | 	// Ignore CR
            ICRNL);		// Map CR to NL

        // ****** c_oflag field setup (Опции вывода)
        // Когда опция OPOST сброшена, все остальные биты опций в поле c_oflag игнорируются.
        options.c_oflag &= ~OPOST;

        tcsetattr(_psp->f, TCSANOW, &options);

        if((r = stream_posix_serial_set_baudrate(_psp, _baudrate))) {
            fprintf(stderr, "Error with posix_serial_port_set_baudrate() call: %m (%d)\n", r);
            break;
        }

        _psp->read_event = event_new(_ev_base,
                                     _psp->f,
                                     EV_READ | EV_PERSIST,
                                     fd_event,
                                     (void*)_psp);

        if(!_psp->read_event) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            r = -1;
            break;
        }

        _psp->write_event = event_new(_ev_base,
                                      _psp->f,
                                      EV_WRITE | EV_PERSIST,
                                      fd_event,
                                      (void*)_psp);

        if(!_psp->write_event) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            r = -1;
            break;
        }

        if((r = event_add(_psp->read_event, NULL))) {
            fprintf(stderr, "Error with event_add() call: %m\n");
            break;
        }

        return 0;

    } while(0);

    stream_posix_serial_close(_psp);

    return errno == 0 ? r : errno;
}

void stream_posix_serial_close(struct stream_posix_serial_t* _psp) {
    if(_psp->read_event) {
        event_del(_psp->read_event);
        event_free(_psp->read_event);
        _psp->read_event = NULL;
    }

    if(_psp->write_event) {
        event_del(_psp->write_event);
        event_free(_psp->write_event);
        _psp->write_event = NULL;
    }

    if(_psp->f != -1) {
        close(_psp->f);
        _psp->f = -1;
    }
}

#define GSBB_BAUD_CASE(_baud_)	case _baud_: return B##_baud_;

static speed_t posix_serial_port_get_speedt_by_baudrate(unsigned int _baudrate){
    switch(_baudrate) {
        GSBB_BAUD_CASE(0)
        GSBB_BAUD_CASE(50)
        GSBB_BAUD_CASE(75)
        GSBB_BAUD_CASE(110)
        GSBB_BAUD_CASE(134)
        GSBB_BAUD_CASE(150)
        GSBB_BAUD_CASE(200)
        GSBB_BAUD_CASE(300)
        GSBB_BAUD_CASE(600)
        GSBB_BAUD_CASE(1200)
        GSBB_BAUD_CASE(1800)
        GSBB_BAUD_CASE(2400)
        GSBB_BAUD_CASE(4800)
        GSBB_BAUD_CASE(9600)
        GSBB_BAUD_CASE(19200)
        GSBB_BAUD_CASE(38400)
        GSBB_BAUD_CASE(57600)
        GSBB_BAUD_CASE(115200)
        GSBB_BAUD_CASE(230400)
        GSBB_BAUD_CASE(460800)
        GSBB_BAUD_CASE(500000)
        GSBB_BAUD_CASE(576000)
        GSBB_BAUD_CASE(921600)
        GSBB_BAUD_CASE(1000000)
        GSBB_BAUD_CASE(1152000)
        GSBB_BAUD_CASE(1500000)
        GSBB_BAUD_CASE(2000000)
        GSBB_BAUD_CASE(2500000)
        GSBB_BAUD_CASE(3000000)
        GSBB_BAUD_CASE(3500000)
        GSBB_BAUD_CASE(4000000)
        default: return -1;
    }
}

int stream_posix_serial_set_baudrate(struct stream_posix_serial_t* _psp,
                                   unsigned int _baudrate) {
    struct termios tt;
    int r;
    speed_t speed;

    if((r = tcgetattr(_psp->f, &tt)))
        return r;

    speed = posix_serial_port_get_speedt_by_baudrate(_baudrate);

    if((r = cfsetspeed(&tt, speed)))
       return r;

    //cfsetispeed(&tt, speed);
    //cfsetospeed(&tt, speed);

    if((r = tcsetattr(_psp->f, TCSANOW, &tt)))
       return r;

    return 0;
}

int stream_posix_serial_set_blocking(struct stream_posix_serial_t* _psp,
                                   int _blocking_enable) {
    if(_blocking_enable) {
        return fcntl(_psp->f, F_SETFL, 0);
    }
    else {
        // Опция FNDELAY указывает функции read возвращать 0
        // если нет символов доступных для чтения из последовательного порта.
        return fcntl(_psp->f, F_SETFL, FNDELAY);
    }
}
