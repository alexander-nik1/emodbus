
#include <emodbus/base/modbus_errno.h>
#include <emodbus/base/add/container_of.h>
#include "rtu_via_tty.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static void modbus_rtu_on_char(struct emb_rtu_t* _emb) {
    if(_emb) {
        struct rtu_via_tty_t* _this =
                container_of(_emb, struct rtu_via_tty_t, rtu);
    }
//    event_add(_this->char_timeout_timer, &_this->char_pause);
}

static int read_from_port(struct emb_rtu_t* _mbt,
                          void* _p_buf,
                          unsigned int _buf_size)
{
    if(_mbt) {
        struct rtu_via_tty_t* _this =
                container_of(_mbt, struct rtu_via_tty_t, rtu);
        return read(_this->fd, _p_buf, _buf_size);
    }
    return 0;
}

static int write_to_port(struct emb_rtu_t* _mbt,
                         const void* _p_data,
                         unsigned int _sz_to_write,
                         unsigned int* _wrote)
{
    if(_mbt) {
        struct rtu_via_tty_t* _this =
                container_of(_mbt, struct rtu_via_tty_t, rtu);

        int counter = _sz_to_write;
        const uint8_t* p_data = (uint8_t*)_p_data;

        while(counter > 0) {
            int res = write(_this->fd, p_data, counter);
            if(res > 0) {
                counter -= res;
                if(_wrote)
                    *_wrote += res;
            }
            else if(counter < 0) {
                fprintf(stderr, "%s(): Error with write() call: %m\n", __FUNCTION__);
                return -1;
            }
        }
        return _sz_to_write;
    }
    return -EINVAL;
}

void rtu_via_tty_init(struct rtu_via_tty_t* _ctx,
                     const char* _tty_name,
                     unsigned int _baudrate)
{
    if(_ctx) {
        _ctx->tty_name = _tty_name;
        _ctx->baudrate = _baudrate;
        _ctx->fd = -1;
        _ctx->is_opened = 0;

        _ctx->rtu.rx_buffer = _ctx->rx_buf;
        _ctx->rtu.tx_buffer = _ctx->tx_buf;
        _ctx->rtu.rx_buf_size = MAX_PDU_SIZE;
        _ctx->rtu.tx_buf_size = MAX_PDU_SIZE;

        _ctx->rtu.emb_rtu_on_char = modbus_rtu_on_char;

        _ctx->rtu.read_from_port = read_from_port;
        _ctx->rtu.write_to_port = write_to_port;

        emb_rtu_initialize(&_ctx->rtu);
    }
}

int rtu_via_tty_open(struct rtu_via_tty_t* _ctx)
{
    struct termios options;

    do {

        if(!_ctx)
            return -EINVAL;

        _ctx->fd = open(_ctx->tty_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if(_ctx->fd < 0) {
            fprintf(stderr, "%s: Error while open \"%s\" serial port: %m\n", __FUNCTION__, _ctx->tty_name);
            break;
        }

        tcgetattr(_ctx->fd, &options);

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

        tcsetattr(_ctx->fd, TCSANOW, &options);

        if(rtu_via_tty_set_baudrate(_ctx, _ctx->baudrate)) {
            fprintf(stderr, "%s: Error with serial_port_set_baudrate() call: %m\n", __FUNCTION__);
            break;
        }
        _ctx->is_opened = 1;
        return 0;

    } while(0);

    rtu_via_tty_close(_ctx);

    return -1;
}

void rtu_via_tty_close(struct rtu_via_tty_t* _ctx)
{
    if(_ctx) {
        _ctx->is_opened = 0;
        close(_ctx->fd);
    }
}

#define GSBB_BAUD_CASE(_baud_)	case _baud_: return B##_baud_;

static speed_t posix_serial_port_get_speedt_by_baudrate(unsigned int _baudrate)
{
    switch(_baudrate) {
#ifdef B0
        GSBB_BAUD_CASE(0)
#endif
#ifdef B50
        GSBB_BAUD_CASE(50)
#endif
#ifdef B75
        GSBB_BAUD_CASE(75)
#endif
#ifdef B110
        GSBB_BAUD_CASE(110)
#endif
#ifdef B134
        GSBB_BAUD_CASE(134)
#endif
#ifdef B150
        GSBB_BAUD_CASE(150)
#endif
#ifdef B200
        GSBB_BAUD_CASE(200)
#endif
#ifdef B300
        GSBB_BAUD_CASE(300)
#endif
#ifdef B600
        GSBB_BAUD_CASE(600)
#endif
#ifdef B1200
        GSBB_BAUD_CASE(1200)
#endif
#ifdef B1800
        GSBB_BAUD_CASE(1800)
#endif
#ifdef B2400
        GSBB_BAUD_CASE(2400)
#endif
#ifdef B4800
        GSBB_BAUD_CASE(4800)
#endif
#ifdef B9600
        GSBB_BAUD_CASE(9600)
#endif
#ifdef B19200
        GSBB_BAUD_CASE(19200)
#endif
#ifdef B38400
        GSBB_BAUD_CASE(38400)
#endif
#ifdef B57600
        GSBB_BAUD_CASE(57600)
#endif
#ifdef B115200
        GSBB_BAUD_CASE(115200)
#endif
#ifdef B230400
        GSBB_BAUD_CASE(230400)
#endif
#ifdef B460800
        GSBB_BAUD_CASE(460800)
#endif
#ifdef B500000
        GSBB_BAUD_CASE(500000)
#endif
#ifdef B576000
        GSBB_BAUD_CASE(576000)
#endif
#ifdef B921600
        GSBB_BAUD_CASE(921600)
#endif
#ifdef B1000000
        GSBB_BAUD_CASE(1000000)
#endif
#ifdef B1152000
        GSBB_BAUD_CASE(1152000)
#endif
#ifdef B1500000
        GSBB_BAUD_CASE(1500000)
#endif
#ifdef B2000000
        GSBB_BAUD_CASE(2000000)
#endif
#ifdef B2500000
        GSBB_BAUD_CASE(2500000)
#endif
#ifdef B3000000
        GSBB_BAUD_CASE(3000000)
#endif
#ifdef B3500000
        GSBB_BAUD_CASE(3500000)
#endif
#ifdef B4000000
        GSBB_BAUD_CASE(4000000)
#endif
        default: return -1;
    }
}

int rtu_via_tty_set_baudrate(struct rtu_via_tty_t *_ctx,
                             unsigned int _baudrate)
{
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

        _ctx->baudrate = _baudrate;

         return 0;
     }
     else
         return -EINVAL;
 }

#define DBG(...) // printf(__VA_ARGS__)

int rtu_via_tty_receive_pdu(struct rtu_via_tty_t* _ctx,
                            int _timeout_msec)
{
    if(_ctx && _ctx->is_opened) {

        struct timeval tv;
        int ret;

        fd_set rfds;

        /// TODO COMPARE SYSTEM TIME AND WAIT FOR remainder of 3.5 IF NEED (!)

        // ***************************************
        // Waiting for the first symbol

        FD_ZERO(&rfds);

        FD_SET(_ctx->fd, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = _timeout_msec * 1000;

        ret = select(_ctx->fd+1, &rfds, NULL, NULL, _timeout_msec >= 0 ? &tv : NULL);
        if(ret > 0) {   // one or more events is happen
            DBG("Read event (first symbol)\n");
            emb_rtu_port_event(&_ctx->rtu, emb_rtu_data_received_event);
        }
        else if(!ret) { // timeout
            DBG("RECEIVE Timeout event\n");
            return -modbus_timeout;
        }
        else { // error
            fprintf(stderr, "%s: select() returns error: %m\n", __FUNCTION__);
            return -errno;
        }

        // ***************************************
        // Waiting for the next symbols, until a symbol timeout is happens

        while(1) {
            FD_ZERO(&rfds);

            FD_SET(_ctx->fd, &rfds);

            tv.tv_sec = 0;
            tv.tv_usec = 1000000 * 35 / _ctx->baudrate;

            ret = select(_ctx->fd+1, &rfds, NULL, NULL, &tv);
            if(ret > 0) { // we have the some data to read
                DBG("Read event\n");
                emb_rtu_port_event(&_ctx->rtu, emb_rtu_data_received_event);
            }
            else if(!ret) { // timeout of 3.5 symbols
                DBG("RD Timeout event (end of packet)\n");

                /// TODO GET SYSTEM TIME HERE
                emb_rtu_on_char_timeout(&_ctx->rtu);

                return 0;
            }
            else { // error
                fprintf(stderr, "%s: select() returns error: %m\n", __FUNCTION__);
                emd_rtu_reset_rx(&_ctx->rtu);
                return -errno;
            }
        }
    }
    return -EINVAL;
}

int rtu_via_tty_send_pdu(struct rtu_via_tty_t* _ctx,
                         int _timeout_msec)
{
    if(_ctx && _ctx->is_opened) {
        struct timeval tv;
        int ret;

        fd_set wfds;

        /// TODO COMPARE SYSTEM TIME AND WAIT FOR remainder of 3.5 IF NEED (!)

        FD_ZERO(&wfds);

        FD_SET(_ctx->fd, &wfds);

        tv.tv_sec = 0;
        tv.tv_usec = _timeout_msec * 1000;

        ret = select(_ctx->fd+1, NULL, &wfds, NULL, _timeout_msec >= 0 ? &tv : NULL);
        if(ret > 0) {   // we are wrote something
            DBG("Write event\n");
            emb_rtu_port_event(&_ctx->rtu, emb_rtu_tx_buf_empty_event);
        }
        else if(!ret) { // timeout
            DBG("Writing: Timeout event\n");
            return -modbus_timeout;
        }
        else { // error
            fprintf(stderr, "%s: select() returns error: %m\n", __FUNCTION__);
            emd_rtu_reset_tx(&_ctx->rtu);
            return -errno;
        }
    }


    /// TODO GET SYSTEM TIME HERE
    return 0;
}
