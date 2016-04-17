
#ifndef EMODBUS_HPP
#define EMODBUS_HPP

#include <emodbus/client/client.h>
#include <vector>
#include <list>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>

#include <emodbus/server/server.h>
#include <emodbus/server/holdings.h>
#include <emodbus/server/file.h>

namespace emb {

// *******************************************************************************
// pdu_t

class pdu_t : public emb_pdu_t {
public:
    pdu_t();

    pdu_t(unsigned int _sz);

    void resize(unsigned int _size);

    operator emb_pdu_t* ();

    operator emb_const_pdu_t* () const;

private:
    std::vector<char> buffer;
};

// *******************************************************************************
// read_hold_regs_t

class read_regs_t {
public:
    read_regs_t();

    void build_req(uint16_t _starting_address, uint16_t _quantity);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
    uint16_t get_answer_reg(uint16_t _offset) const;
    uint16_t get_answer_quantity() const;

    pdu_t req, ans;
};

// *******************************************************************************
// write_reg_t

class write_reg_t {
public:
    write_reg_t();

    void build_req(uint16_t _address, uint16_t _value);

    uint16_t get_req_address() const;
    uint16_t get_req_value() const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_value() const;

    pdu_t req, ans;
};

// *******************************************************************************
// write_regs_t

class write_regs_t {
public:
    write_regs_t();

    void build_req(uint16_t _address, uint16_t _quantity, const uint16_t* _data);

    uint16_t get_req_address() const;
    uint16_t get_req_quantity() const;
    uint16_t get_req_data(uint16_t _offset) const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_quantity() const;

    pdu_t req, ans;
};

// *******************************************************************************
// write_mask_reg_t

class write_mask_reg_t {
public:
    write_mask_reg_t();

    void build_req(uint16_t _address,
                   uint16_t _and_mask,
                   uint16_t _or_mask);

    uint16_t get_req_address() const;
    uint16_t get_req_and_mask() const;
    uint16_t get_req_or_mask() const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_and_mask() const;
    uint16_t get_answer_or_mask() const;

    pdu_t req, ans;
};

// *******************************************************************************
// read_file_record_t

class read_file_record_t {
public:
    read_file_record_t();

    typedef emb_read_file_req_t sub_req_t;

    void build_req(const sub_req_t* _reqs, int _reqs_number);

    pdu_t req, ans;
};

// *******************************************************************************
// write_file_record_t

class write_file_record_t {
public:
    write_file_record_t();

    typedef emb_write_file_req_t sub_req_t;

    void build_req(const sub_req_t* _reqs, int _reqs_number);

    pdu_t req, ans;
};

// *******************************************************************************
// read_fifo_t

class read_fifo_t {
public:
    read_fifo_t();

    void build_req(uint16_t _starting_address);

    uint16_t get_answer_regs_count() const;
    uint16_t get_answer_data(uint16_t _offset) const;

    pdu_t req, ans;
};

// *******************************************************************************
// sync_client_t

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

protected:

    static void emb_on_response(struct emb_client_request_t* _req, int _slave_addr);

    static void emb_on_error(struct emb_client_request_t* _req, int _errno);

    struct emb_client_request_t req;
    static struct emb_client_req_procs_t procs;
    struct emb_client_t client;

    int result;
};

// *******************************************************************************
// aync_client_t

class aync_client_t {
public:
    aync_client_t();

    struct claabacker_t {
        virtual void emb_client_on_response(struct emb_client_request_t* _req, int _req_id, int _slave_addr) =0;
        virtual void emb_client_on_error(struct emb_client_request_t* _req, int _req_id, int _errno) =0;
    };

    int do_request(int _server_addr,
                   int _req_id,
                   emb_const_pdu_t* _request,
                   emb_pdu_t *_response,
                   claabacker_t* _callbacker);

    void set_proto(struct emb_protocol_t* _proto);

    void answer_timeout();

private:
    static void emb_on_response(struct emb_client_request_t* _req, int _slave_addr);

    static void emb_on_error(struct emb_client_request_t* _req, int _errno);

    struct emb_client_request_t req;
    struct emb_client_t client;
    static struct emb_client_req_procs_t procs;
    claabacker_t* curr_callbacker;
    int curr_req_id;
};

// *******************************************************************************
// server_holdings_t

class server_t;

class server_holdings_t {
    friend class server_t;
public:
    server_holdings_t();
    server_holdings_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);
    virtual uint8_t on_read_regs(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues);
    virtual uint8_t on_write_regs(uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues);

private:
    void set_funcs();
    static uint8_t read_regs(struct emb_srv_holdings_t* _rr,
                             uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues);
    static uint8_t write_regs(struct emb_srv_holdings_t* _rr,
                              uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues);

    struct emb_srv_holdings_t h;
};

// *******************************************************************************
// server_file_t

class server_file_t {
    friend class server_t;
public:
    server_file_t();
    server_file_t(uint16_t _fileno/*, uint16_t _start, uint16_t _size*/);

    void set_fileno(uint16_t _fileno);
//    void set_start(uint16_t _start);
//    void set_size(uint16_t _size);
    virtual uint8_t on_read_file(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues);
    virtual uint8_t on_write_file(uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues);

private:
    void set_funcs();
    static uint8_t read_file(struct emb_srv_file_t* _f,
                             uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues);
    static uint8_t write_file(struct emb_srv_file_t* _f,
                              uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues);

    struct emb_srv_file_t f;
};

// *******************************************************************************
// server_t

class super_server_t;

class server_t {
    friend class super_server_t;

    struct function_t {
        uint8_t func_no;
        emb_srv_function_t func;
    };

public:
    server_t(int _address);

    virtual int addr() const;

    bool add_function(uint8_t _func_no, emb_srv_function_t _func);

    bool add_holdings(server_holdings_t& _holdings);
    bool add_file(server_file_t& _file);

private:
    static emb_srv_function_t get_function(struct emb_server_t* _srv, uint8_t _func);

    static struct emb_srv_holdings_t* get_holdings(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_file_t* get_file(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/);

    typedef std::vector<function_t>::iterator func_iter;
    std::vector<function_t> functions;

    typedef std::vector<server_holdings_t*>::iterator holdnigs_iter;
    std::vector<server_holdings_t*> holdings;

    typedef std::vector<server_file_t*>::iterator files_iter;
    std::vector<server_file_t*> files;

protected:
    int address;

private:
    struct emb_server_t srv;
};

// *******************************************************************************
// super_server_t

class super_server_t {
public:
    super_server_t();
    void set_proto(struct emb_protocol_t* _proto);

    bool add_server(server_t& _srv);

private:
    static struct emb_server_t* _get_server(struct emb_super_server_t* _ssrv,
                                            uint8_t _address);
    typedef std::vector<server_t*>::iterator srv_iter;
    std::vector<server_t*> servers;

    struct emb_super_server_t ssrv;
    emb::pdu_t rx_pdu,tx_pdu;
};

}; // namespace emb

#endif // EMODBUS_HPP
