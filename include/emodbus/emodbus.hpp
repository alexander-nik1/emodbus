
#ifndef EMODBUS_HPP
#define EMODBUS_HPP

#include <emodbus/client/client.h>
#include <vector>

namespace emb {

class pdu_t : public emb_pdu_t {
public:
    pdu_t(unsigned int _sz);

    void resize(unsigned int _size);

    operator emb_pdu_t* ();

    operator emb_const_pdu_t* () const;

private:
    std::vector<char> buffer;
};



class sync_client_t {
public:
    sync_client_t();

    int do_request(int _server_addr,
                    unsigned int _timeout,
                    emb_const_pdu_t* _request,
                    emb_pdu_t *_response);

    void set_proto(struct emb_protocol_t* _proto);

protected:

    virtual int emb_client_start_wait(unsigned int _timeout) = 0;
    virtual void emb_client_stop_wait() = 0;

private:

    static void emb_on_response(struct emb_client_request_t* _req, int _slave_addr);

    static void emb_on_error(struct emb_client_request_t* _req, int _errno);

    struct emb_client_request_t req;
    static struct emb_client_req_procs_t procs;
    struct emb_client_t client;

    int result;
};

}; // namespace emb

#endif // EMODBUS_HPP
