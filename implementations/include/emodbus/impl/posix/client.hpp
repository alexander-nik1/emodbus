
#ifndef EMB_POSIX_CLIENT_HPP
#define EMB_POSIX_CLIENT_HPP

#include <emodbus/emodbus.hpp>
#include <event.h>
#include <pthread.h>

namespace emb {

class posix_sync_client_t : public client::client_t {
public:
    posix_sync_client_t(struct event_base* _eb);

    ~posix_sync_client_t();

    void init_timers(struct event_base* _eb);
    int do_transaction(int _server_addr,
                       unsigned int _timeout,
                       client::transaction_t &_transaction);

private:
    virtual int emb_sync_client_start_wait(unsigned int _timeout);
    virtual void emb_on_response(int _slave_addr);
    virtual void emb_on_error(int _slave_addr, int _errno);
    static void timeout_cb(evutil_socket_t, short, void * _arg);

    bool is_timeout;
    pthread_mutex_t mutex;
    struct event* timeout_timer;
};

} // namespace emb

#endif // EMB_POSIX_CLIENT_HPP
