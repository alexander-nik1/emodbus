
#include <emodbus/implementations/posix/serial-rtu.hpp>
#include <emodbus/base/add/container_of.h>
#include <emodbus/base/modbus_pdu.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

serial_rtu_t::serial_rtu_t()
    : char_timeout_timer(NULL)
    , read_event(NULL)
    , write_event(NULL)
{ }

serial_rtu_t::~serial_rtu_t() {
    close();
}

int serial_rtu_t::open(event_base *_base,
                       const char* _dev_name,
                       unsigned int _baudrate) {

    int res;

    printf("%s: opening serial port(%s), baud:%d\n", __PRETTY_FUNCTION__, _dev_name, _baudrate);

    struct termios options;

    do {
        fd = ::open(_dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
        if(fd < 0) {
            fprintf(stderr, "Error while open \"%s\" serial port: %m\n", _dev_name);
            res = -1;
            break;
        }

        tcgetattr(fd, &options);

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

        tcsetattr(fd, TCSANOW, &options);

        if((res = set_baudrate(_baudrate))) {
            fprintf(stderr, "Error with posix_serial_port_set_baudrate() call: %m (%d)\n", res);
            break;
        }

        read_event = event_new(_base,
                               fd,
                               EV_READ | EV_PERSIST,
                               fd_event,
                               this);

        if(!read_event) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            res = -1;
            break;
        }

        write_event = event_new(_base,
                                fd,
                                EV_WRITE | EV_PERSIST,
                                fd_event,
                                this);

        if(!write_event) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            res = -1;
            break;
        }

        if((res = event_add(read_event, NULL))) {
            fprintf(stderr, "Error with event_add() call: %m\n");
            break;
        }

        char_pause.tv_sec = 0;
      //  enum { pause = 100 };
        char_pause.tv_usec = 1000 * 10; //(1000 * 1000) / (_baudrate / pause);

        char_timeout_timer = event_new(_base,
                                       -1,
                                       EV_TIMEOUT/* | EV_PERSIST*/,
                                       on_timer,
                                       this);
        if(!char_timeout_timer) {
            fprintf(stderr, "Error with event_new() call: %m\n");
            res = -1;
            break;
        }
    } while(0);

    if(res)
        return res;

    rtu_init();

    return 0;
}

void serial_rtu_t::close() {
    if(char_timeout_timer) {
        event_del(char_timeout_timer);
        event_free(char_timeout_timer);
        char_timeout_timer = NULL;
    }

    if(read_event) {
        event_del(read_event);
        event_free(read_event);
        read_event = NULL;
    }

    if(write_event) {
        event_del(write_event);
        event_free(write_event);
        write_event = NULL;
    }

    if(fd != -1) {
        ::close(fd);
        fd = -1;
    }
}

struct emb_protocol_t* serial_rtu_t::get_proto() {
    return &modbus_rtu.proto;
}

void serial_rtu_t::rtu_init() {
    rx_buffer.resize(MAX_PDU_SIZE);
    tx_buffer.resize(MAX_PDU_SIZE);

    modbus_rtu.rx_buffer = &rx_buffer[0];
    modbus_rtu.tx_buffer = &tx_buffer[0];
    modbus_rtu.rx_buf_size = rx_buffer.size();
    modbus_rtu.tx_buf_size = tx_buffer.size();
    modbus_rtu.emb_rtu_on_char = modbus_rtu_on_char;

    modbus_rtu.read_from_port = read_from_port;
    modbus_rtu.write_to_port = write_to_port;

    emb_rtu_initialize(&modbus_rtu);
}

void serial_rtu_t::modbus_rtu_on_char(struct emb_rtu_t* _emb) {
    serial_rtu_t* _this = container_of(_emb, serial_rtu_t, modbus_rtu);
    event_add(_this->char_timeout_timer, &_this->char_pause);
}

void serial_rtu_t::on_timer(evutil_socket_t fd, short what, void *arg) {
    serial_rtu_t* _this = (serial_rtu_t*)arg;
    emb_rtu_on_char_timeout(&_this->modbus_rtu);
}

int serial_rtu_t::read_from_port(struct emb_rtu_t* _mbt,
                                          void* _p_buf,
                                          unsigned int _buf_size) {

    serial_rtu_t* _this = container_of(_mbt, serial_rtu_t, modbus_rtu);
    return ::read(_this->fd, _p_buf, _buf_size);
}

int serial_rtu_t::write_to_port(struct emb_rtu_t* _mbt,
                                         const void* _p_data,
                                         unsigned int _sz_to_write) {
    serial_rtu_t* _this = container_of(_mbt, serial_rtu_t, modbus_rtu);

    if(!_sz_to_write) {
        event_del(_this->write_event);
        return 0;
    }

    int r;
    const int res = ::write(_this->fd, _p_data, _sz_to_write);
    fsync(_this->fd);

    if((r = event_add(_this->write_event, NULL)))
        fprintf(stderr, "Error with event_add() call: %m (%d)\n", r);

    return res;
}

void serial_rtu_t::fd_event(evutil_socket_t fd, short what, void *arg) {
    serial_rtu_t* _this = (serial_rtu_t*)arg;

    if(what & EV_READ)
        emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_data_received_event);

    if(what & EV_WRITE)
        emb_rtu_port_event(&_this->modbus_rtu, emb_rtu_tx_buf_empty_event);
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

int serial_rtu_t::set_baudrate(unsigned int _baudrate) {
    struct termios tt;
    int r;
    speed_t speed;

    if((r = tcgetattr(fd, &tt)))
        return r;

    speed = posix_serial_port_get_speedt_by_baudrate(_baudrate);

    if((r = cfsetspeed(&tt, speed)))
       return r;

    //cfsetispeed(&tt, speed);
    //cfsetospeed(&tt, speed);

    if((r = tcsetattr(fd, TCSANOW, &tt)))
       return r;

    return 0;
}
