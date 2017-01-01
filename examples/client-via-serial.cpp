
#include <emodbus/impl/posix/client.hpp>
#include <emodbus/impl/posix/mb-rtu-via-serial.h>
#include <emodbus/impl/posix/mb-rtu-via-tcp-client.h>
#include <emodbus/impl/posix/dumper.h>
#include <emodbus/base/modbus_errno.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <exception>

#include <event2/thread.h>

/***************************************************************************************************/
/*                                                                                                 */
/*                            SYNCHRONOUS CLIENT EXAMPLE (C++ Version)                             */
/*                                                                                                 */
/***************************************************************************************************/

/***************************************************************************************************/
// Synchronous client
emb::posix_sync_client_t client(NULL);


static char s_zSerialPortName[ 64 ];
static uint32_t s_uSerialBaud  = 115200;
/***************************************************************************************************/
// Thread for a transport-level operations.
// This thread works with a physical port.
void* thr_proc(void* p) {
    evthread_use_pthreads();
    struct event_base *base = event_base_new();

    // Create the RTU transport
    struct emb_rtu_via_serial_t* rtu = emb_rtu_via_serial_create(base, 10, s_zSerialPortName, s_uSerialBaud);
//     struct emb_rtu_via_tcp_client_t* rtu = emb_rtu_via_tcp_client_create(base, 50, "10.1.1.144", 4009);

    // Enable a packets dumping
//     emb_rtu_via_tcp_client_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;
    emb_rtu_via_serial_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;
    emb_posix_dumping_stream = stdout;
    emb_posix_dumper_enable_rx_tx();

    // Bind the client to the RTU
//     client.set_transport(emb_rtu_via_tcp_client_get_transport(rtu));
    client.set_transport(emb_rtu_via_serial_get_transport(rtu));

    // Because, a client was created befire the base,
    // we need init the client's timers here.
    // NOTE: if you are set valid base to constructor of emb::posix_sync_client_t,
    // then you are NOT need to do this call.
    client.init_timers(base);

    // Libevent's loop
    event_base_dispatch(base);

    // Destroy the RTU
//     emb_rtu_via_tcp_client_destroy(rtu);
    emb_rtu_via_serial_destroy(rtu);
}

/***************************************************************************************************/
// Main
int main( int argc, char *argv[] ) {

    if ( argc < 6 )
    {
	std::cerr << "Not enough parameters. Use " << argv[ 0 ] << ":" << std::endl;	
	std::cerr << "\t<serial port name> - Absolut path to COM-port file name;" << std::endl;
	std::cerr << "\t<serial baud> - Baud rate for specified COM-port;" << std::endl;	
	std::cerr << "\t<slave address> - Modbus-slave address;" << std::endl;
	std::cerr << "\t<base register> - Base register address of request;" << std::endl;
	std::cerr << "\t<registers count> - Register quantity of modbus request." << std::endl;
	return 1;
    }
    
    strcpy( s_zSerialPortName, argv[ 1 ] );
    
    s_uSerialBaud  	 = atoi( argv[ 2 ] );
    uint16_t mbSlaveAddr = atoi( argv[ 3 ] );
    uint16_t mbBaseReg   = atoi( argv[ 4 ] );
    uint16_t mbRegsCount = atoi( argv[ 5 ] );
    
    std::cerr << "========================" << std::endl;
    std::cerr << "Serial port: " << s_zSerialPortName << std::endl;
    std::cerr << "Serial baud: " << s_uSerialBaud << std::endl;
    std::cerr << "Slave addr: " << mbSlaveAddr << std::endl;
    std::cerr << "Base register: " << mbBaseReg << std::endl;
    std::cerr << "Registers quantity: " << mbRegsCount << std::endl;
    std::cerr << "========================" << std::endl;
    
    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, NULL);
    sleep(1);   // We must wait here for all initialization is made in started thread.

//     emb::client::proxy_t device1(&client, 0x02);
//     emb::client::proxy_t device2(&client, 0x30);

//     device1.set_timeout(100);
//     device2.set_timeout(1000);

    emb::client::read_holding_regs_t rr;

    for(int i=0; i<1000; ++i) {

        try {
            rr.build_req(mbBaseReg, mbRegsCount);

            int res = client.do_transaction(mbSlaveAddr, 1000, rr);
            if(!res)
                printf("x = 0x%04X\n", rr.get_answer_reg(0));
            else {
                std::cerr << "Exception: (" << res << ") "
                          << emb_strerror(-res) << std::endl;
            }
        }
        catch(std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        catch(int & e) {
            std::cerr << "Exception: (" << e << ") " << emb_strerror(-e) << std::endl;
        }

        sleep(1);
    }

    std::cout << "RX bytes: " << emb_posix_dumper_rx_bytes() << std::endl;
    std::cout << "TX bytes: " << emb_posix_dumper_tx_bytes() << std::endl;
}
