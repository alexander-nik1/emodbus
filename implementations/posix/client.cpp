
#include <emodbus/impl/posix/client.hpp>

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

namespace emb {

posix_sync_client_t::posix_sync_client_t(struct event_base* _eb)
    : timeout_timer(NULL)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_trylock(&mutex);

    client::client_t::set_sync(true);

    init_timers(_eb);
}

posix_sync_client_t::~posix_sync_client_t() {
    if(timeout_timer) {
        event_free(timeout_timer);
        timeout_timer = NULL;
    }
}

void posix_sync_client_t::init_timers(struct event_base* _eb) {
    if(_eb && !timeout_timer) {
        timeout_timer = event_new(_eb, -1, EV_TIMEOUT, timeout_cb, this);
    }
}

int posix_sync_client_t::do_transaction(int _server_addr,
                             unsigned int _timeout,
                             client::transaction_t &_transaction) {
    int r;
    while((r = client::client_t::do_transaction(_server_addr, _timeout, _transaction)) == -16) {
        usleep(1);
    }
    return r;
}

int posix_sync_client_t::emb_sync_client_start_wait(unsigned int _timeout) {
    int res;
    is_timeout = false;

    struct timeval timeout_time;

    timeout_time.tv_sec = 0;
    timeout_time.tv_usec = _timeout * 1000;

    if(timeout_timer) {
        event_add(timeout_timer, &timeout_time);
    }
    else {
        fprintf(stderr, "%s: can't keep timeouts, timeout_timer is NULL!\n", __PRETTY_FUNCTION__);
        return -1;
    }

//    pthread_mutex_trylock(&mutex);

    res = pthread_mutex_lock(&mutex);

    event_del(timeout_timer);

    if(is_timeout) {
        return 1;
    }
    else {
        return result;
    }
}

void posix_sync_client_t::emb_on_response(int _slave_addr) {
    is_timeout = false;
    result = 0;
    pthread_mutex_unlock(&mutex);
}

void posix_sync_client_t::emb_on_error(int _slave_addr, int _errno) {
    is_timeout = false;
    result = _errno;
    pthread_mutex_unlock(&mutex);
}

void posix_sync_client_t::timeout_cb(evutil_socket_t, short, void * _arg) {
    posix_sync_client_t* _this = (posix_sync_client_t*)_arg;
    _this->is_timeout = true;
    _this->sync_answer_timeout();
    pthread_mutex_unlock(&_this->mutex);
}

} // namespace emb
