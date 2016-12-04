
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
    struct emb_rtu_via_tcp_client_t* rtu = emb_rtu_via_tcp_client_create(base, 50, "10.1.1.144", 4009);

#if EMODBUS_PACKETS_DUMPING
    // Enable a packets dumping
    emb_rtu_via_tcp_client_get_transport(rtu)->flags |= EMB_TRANSPORT_FLAG_DUMD_PAKETS;
    emb_posix_dumping_stream = stdout;
    emb_posix_dumper_enable_rx_tx();
#endif // EMODBUS_PACKETS_DUMPING

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

    emb::client::proxy_t device1(&client, 0x50);
    emb::client::proxy_t device2(&client, 0x30);

    device1.set_timeout(1000);
    device2.set_timeout(1000);

    emb::client::read_holding_regs_t rr;

    for(int i=0; i<1000; ++i) {

        try {
            rr.build_req(1, 1);

            int res = client.do_transaction(80, 1000, rr);
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
#if EMODBUS_PACKETS_DUMPING
    std::cout << "RX bytes: " << emb_posix_dumper_rx_bytes() << std::endl;
    std::cout << "TX bytes: " << emb_posix_dumper_tx_bytes() << std::endl;
#endif // EMODBUS_PACKETS_DUMPING
}
