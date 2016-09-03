
#include <emodbus/implementations/posix/serial-port.h>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_pdu.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct serial_port_t {
    struct event* read_event, *write_event;
    int fd;
    serial_port_notifier_t notifier;
    void* notifier_parameter;
};

static void fd_event(evutil_socket_t _fd, short _what, void *_arg) {
    if(_arg) {

        struct serial_port_t* ctx = (struct serial_port_t*)_arg;

        if(ctx->notifier) {
            if(_what & EV_READ)
                ctx->notifier(ctx->notifier_parameter, serial_port_data_received_event);

            if(_what & EV_WRITE)
                ctx->notifier(ctx->notifier_parameter, serial_port_data_sent_event);
        }
    }
}

struct serial_port_t*
serial_port_create(struct event_base *_base,
                   const char* _dev_name,
                   unsigned int _baudrate) {

    struct termios options;

    struct serial_port_t* sp = NULL;

    do {
        if(!(sp = (struct serial_port_t*)malloc(sizeof(struct serial_port_t))))
            break;

        memset(sp, 0, sizeof(struct serial_port_t));

        sp->fd = open(_dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if(sp->fd < 0) {
            fprintf(stderr, "%s: Error while open \"%s\" serial port: %m\n", __FUNCTION__, _dev_name);
            fflush(stderr);
            break;
        }

        tcgetattr(sp->fd, &options);

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

        tcsetattr(sp->fd, TCSANOW, &options);

        if(serial_port_set_baudrate(sp, _baudrate)) {
            fprintf(stderr, "%s: Error with serial_port_set_baudrate() call: %m\n", __FUNCTION__);
            fflush(stderr);
            break;
        }

        sp->read_event = event_new(_base,
                                   sp->fd,
                                   EV_READ | EV_PERSIST,
                                   fd_event,
                                   sp);
        if(!sp->read_event) {
            fprintf(stderr, "%s: Error with event_new() call: %m\n", __FUNCTION__);
            fflush(stderr);
            break;
        }

        sp->write_event = event_new(_base,
                                    sp->fd,
                                    EV_WRITE | EV_PERSIST,
                                    fd_event,
                                    sp);
        if(!sp->write_event) {
            fprintf(stderr, "%s: Error with event_new() call: %m\n", __FUNCTION__);
            fflush(stderr);
            break;
        }

        if(event_add(sp->read_event, NULL)) {
            fprintf(stderr, "%s: Error with event_add() call: %m\n", __FUNCTION__);
            fflush(stderr);
            break;
        }

        return sp;

    } while(0);

    serial_port_destroy(sp);

    return NULL;
}

void serial_port_destroy(struct serial_port_t* _ctx) {
    if(_ctx) {
        close(_ctx->fd);
        if(_ctx->read_event) {
            event_del(_ctx->read_event);
            event_free(_ctx->read_event);
        }
        if(_ctx->write_event) {
            event_del(_ctx->write_event);
            event_free(_ctx->write_event);
        }
        free(_ctx);
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

int serial_port_set_baudrate(struct serial_port_t* _ctx,
                             unsigned int _baudrate) {
    if(_ctx) {
        struct termios tt;
        int r;
        speed_t speed;

        if((r = tcgetattr(_ctx->fd, &tt)))
            return r;

        speed = posix_serial_port_get_speedt_by_baudrate(_baudrate);

        if((r = cfsetspeed(&tt, speed)))
           return r;

        //cfsetispeed(&tt, speed);
        //cfsetospeed(&tt, speed);

        if((r = tcsetattr(_ctx->fd, TCSANOW, &tt)))
           return r;

        return 0;
    }
    else
        return -EINVAL;
}

int serial_port_read(struct serial_port_t* _ctx,
                     void* _p_buf,
                     unsigned int _buf_size) {

    if(_ctx)
        return read(_ctx->fd, _p_buf, _buf_size);

    return -EINVAL;
}

int serial_port_write(struct serial_port_t* _ctx,
                      const void* _p_data,
                      unsigned int _sz_to_write) {
    if(_ctx) {
        int res, r;

        if(!_sz_to_write) {
            event_del(_ctx->write_event);
            return 0;
        }

        res = write(_ctx->fd, _p_data, _sz_to_write);
        fsync(_ctx->fd);

        if((r = event_add(_ctx->write_event, NULL))) {
            fprintf(stderr, "%s: Error with event_add() call: %m (%d)\n", __FUNCTION__, r);
            fflush(stderr);
        }

        return res;
    }

    return -EINVAL;
}
