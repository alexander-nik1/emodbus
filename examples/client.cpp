
#include <emodbus/impl/posix/client.hpp>
#include <emodbus/impl/posix/mb-rtu-via-serial.h>
#include <emodbus/impl/posix/mb-rtu-via-tcp-client.h>
#include <emodbus/impl/posix/dumper.h>
#include <emodbus/base/modbus_errno.h>

#include <unistd.h>
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

/***************************************************************************************************/
// Thread for a transport-level operations.
// This thread works with a physical port.
void* thr_proc(void* p) {
    evthread_use_pthreads();
    struct event_base *base = event_base_new();

    // Create the RTU transport
    //struct emb_rtu_via_serial_t* rtu = emb_rtu_via_serial_create(base, 10, "/dev/ttyUSB0", 115200);
    struct emb_rtu_via_tcp_client_t* rtu = emb_rtu_via_tcp_client_create(base, 40, "192.168.1.138", 4003);

    // Enable a packets dumping
    emb_rtu_via_tcp_client_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;
    emb_posix_dumping_stream = stdout;
    //emb_posix_dumper_enable_rx_tx();

    // Bind the client to the RTU
    client.set_transport(emb_rtu_via_tcp_client_get_transport(rtu));

    // Because, a client was created befire the base,
    // we need init the client's timers here.
    // NOTE: if you are set valid base to constructor of emb::posix_sync_client_t,
    // then you are NOT need to do this call.
    client.init_timers(base);

    // Libevent's loop
    event_base_dispatch(base);

    // Destroy the RTU
    emb_rtu_via_tcp_client_destroy(rtu);
}

/***************************************************************************************************/
// Main
int main() {

    pthread_t pthr;

    pthread_create(&pthr, NULL, thr_proc, NULL);
    sleep(1);   // We must wait here for all initialization is made in started thread.

    emb::client::proxy_t device1(&client, 0x10);
    emb::client::proxy_t device2(&client, 0x30);

    device1.set_timeout(1000);
    device2.set_timeout(1000);

    for(int i=0; i<1000; ++i) {

        try {

            // Read single 0x0000 register form both (16 and 17) servers
            std::cout << std::uppercase << std::hex;
            std::cout << "REG1: 0x" << (int)device1.holdings[0x0000]
                      << "   "
                      << "REG2: 0x" << (int)device2.holdings[0x0000]
                      << std::endl;

            std::cout << std::dec;

            // Read a 0x0000-0x0007 (eight) registers form server 16
            emb::regs_t regs = device1.holdings[emb::range_t(0x0000, 0x0007)];
            std::cout << "REGS1: ";
            for(emb::regs_t::const_iterator i = regs.begin(); i != regs.end(); ++i)
                std::cout << *i << " ";
            std::cout << std::endl;

            // Read a 0x0000-0x0007 (eight) registers form server 17
            regs = device2.holdings[emb::range_t(0x0020, 0x0027)];
            std::cout << "REGS2: ";
            for(emb::regs_t::const_iterator i = regs.begin(); i != regs.end(); ++i)
                std::cout << *i << " ";
            std::cout << std::endl;
        }
        catch(std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        catch(int & e) {
            std::cerr << "Exception: (" << e << ") " << emb_strerror(-e) << std::endl;
        }
    }

    std::cout << "RX bytes: " << emb_posix_dumper_rx_bytes() << std::endl;
    std::cout << "TX bytes: " << emb_posix_dumper_tx_bytes() << std::endl;
}
