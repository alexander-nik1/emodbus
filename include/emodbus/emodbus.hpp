
#ifndef EMODBUS_HPP
#define EMODBUS_HPP

#include <emodbus/client/client.h>
#include <vector>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>

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

}; // namespace emb

#endif // EMODBUS_HPP
