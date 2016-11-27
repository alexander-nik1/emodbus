
#ifndef EMODBUS_HPP
#define EMODBUS_HPP

#include <emodbus/client/client.h>
#include <vector>
#include <list>
#include <emodbus/client/read_bits.h>
#include <emodbus/client/read_regs.h>
#include <emodbus/client/read_file_record.h>
#include <emodbus/client/write_file_record.h>

#include <emodbus/server/server.h>
#include <emodbus/server/bits.h>
#include <emodbus/server/regs.h>
#include <emodbus/server/file.h>

namespace emb {

// Just overload the << operator for useful usage.
template<typename T>
struct vector : public std::vector<T>
{
    vector& operator << (const T& _v) {
        std::vector<T>::push_back(_v);
        return *this;
    }
};

typedef std::pair<uint16_t, uint16_t> range_t;

typedef vector<bool> coils_t;
typedef coils_t inputs_t;

struct regs_t : public std::vector<uint16_t>
{
    regs_t& operator << (const int& _v);
    regs_t& operator << (const float& _v);

    regs_t& operator >> (int& _v);
    regs_t& operator >> (float& _v);
//    template <typename X>
//    regs_t& operator << (const X& _v) {
//        const uint16_t* p = (uint16_t*)&_v;
//        for(int i=0; i<(sizeof(X)/2); ++i) {
//            std::vector<uint16_t>::push_back(*p++);
//        }
//        return *this;
//    }
};

// *******************************************************************************
/**
 * @brief The pdu_t class
 *
 * Class - container of the one PDU.
 *
 * This class contains a emb_pdu_t instance.
 * Also this class contains a one dynamically
 * allocated buffer for this PDU.
 *
 */
class pdu_t : public emb_pdu_t
{
public:
    pdu_t();
    pdu_t(unsigned int _sz);

    ~pdu_t();

    void resize(unsigned int _size);
    operator emb_pdu_t* ();
    operator emb_const_pdu_t* () const;

private:
    std::vector<char> buffer;
};

namespace client {

// *******************************************************************************
//  #####  #       ###   #####  #   #  #####
//  #      #        #    #      ##  #    #
//  #      #        #    #####  # # #    #
//  #      #        #    #      #  ##    #
//  #####  #####   ###   #####  #   #    #

// *******************************************************************************
/**
 * @brief The transaction_t class
 *
 * Contains a data buffers for a one transaction.
 * And a virtual functions for this transaction:
 * emb_transaction_on_response() called, when a valid response was got.
 * emb_transaction_on_error() called, when an error has occured.
 *
 * This object is a wrapper for struct emb_client_transaction_t.
 *
 * @see emb_client_transaction_t
 */
class transaction_t
{
public:

    transaction_t();

    virtual void emb_transaction_on_response(int _slave_addr);
    virtual void emb_transaction_on_error(int _slave_addr, int _errno);

    pdu_t req, ans;

    operator struct emb_client_transaction_t* ();

private:
    static void emb_trans_on_response_(struct emb_client_transaction_t* _tr,
                                       int _slave_addr);
    static void emb_trans_on_error_(struct emb_client_transaction_t* _tr,
                                    int _slave_addr, int _errno);

    static struct emb_client_req_procs_t procs;
    struct emb_client_transaction_t tr;
};

// *******************************************************************************
/**
 * @brief The transact_base_t class
 *
 * This is a base class for each request.
 * Class provides a transaction_t in two kinds:
 * If a transact_base_t() constructor was called, then
 * this object contains its own instance of transaction_t,
 * in destructor, this instance will destroyed.
 * Otherwise, if a transact_base_t(transaction_t& _tr) constructor
 * was called, then this object is just contains a reference to
 * transaction_t object, that was got in constructor.
 *
 */
class transact_base_t
{
public:
    transact_base_t();
    transact_base_t(transaction_t& _tr);
    ~transact_base_t();

    operator transaction_t& ();
    operator const transaction_t& () const;

private:
    transaction_t* self_tr;

public:
    transaction_t* tr;
};

// *******************************************************************************
// read_bits_t

class read_bits_t : public transact_base_t
{
public:
    read_bits_t();
    read_bits_t(transaction_t& _tr);

    int build_req(enum EMB_RB_TYPE _type,
                  uint16_t _starting_address,
                  uint16_t _quantity);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
    char get_answer_bit(uint16_t _offset) const;
    uint8_t get_answer_byte(uint8_t _offset) const;

    void response_data(bool* _coils, unsigned int _size) const;
};

// *******************************************************************************
// read_coils_t

class read_coils_t : public read_bits_t
{
public:
    read_coils_t();
    read_coils_t(transaction_t& _tr);

    int build_req(uint16_t _starting_address, uint16_t _quantity);
};

// *******************************************************************************
// read_discrete_inputs_t

class read_discrete_inputs_t : public read_bits_t
{
public:
    read_discrete_inputs_t();
    read_discrete_inputs_t(transaction_t& _tr);

    int build_req(uint16_t _starting_address, uint16_t _quantity);
};

// *******************************************************************************
// write_coil_t

class write_coil_t : public transact_base_t
{
public:
    write_coil_t();
    write_coil_t(transaction_t& _tr);

    int build_req(uint16_t _address, bool _value);

    uint16_t get_req_addr() const;
};

// *******************************************************************************
// write_coils_t

class write_coils_t : public transact_base_t
{
public:
    write_coils_t();
    write_coils_t(transaction_t& _tr);

    int build_req(uint16_t _starting_address, uint16_t _quantity,
                  const bool *_pcoils);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
};

// *******************************************************************************
// read_regs_t

class read_regs_t : public transact_base_t
{
public:
    read_regs_t();
    read_regs_t(transaction_t& _tr);

    int build_req(enum EMB_RR_TYPE _subtype,
                  uint16_t _starting_address, uint16_t _quantity);

    uint16_t get_req_starting_addr() const;
    uint16_t get_req_quantity() const;
    uint16_t get_answer_reg(uint16_t _offset) const;
    uint16_t get_answer_quantity() const;

    void get_answer_regs(uint16_t* _p_data, uint16_t _offset, uint16_t _n_regs);
    void get_answer_regs(regs_t& _res, uint16_t _offset, uint16_t _n_regs);
};

// *******************************************************************************
// read_holding_regs_t

class read_holding_regs_t : public read_regs_t
{
public:
    read_holding_regs_t();
    read_holding_regs_t(transaction_t& _tr);

    int build_req(uint16_t _starting_address, uint16_t _quantity);
};

// *******************************************************************************
// read_input_regs_t

class read_input_regs_t : public read_regs_t
{
public:
    read_input_regs_t();
    read_input_regs_t(transaction_t& _tr);

    int build_req(uint16_t _starting_address, uint16_t _quantity);
};

// *******************************************************************************
// write_reg_t

class write_reg_t : public transact_base_t
{
public:
    write_reg_t();
    write_reg_t(transaction_t& _tr);

    int build_req(uint16_t _address, uint16_t _value);

    uint16_t get_req_address() const;
    uint16_t get_req_value() const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_value() const;
};

// *******************************************************************************
// write_regs_t

class write_regs_t : public transact_base_t
{
public:
    write_regs_t();
    write_regs_t(transaction_t& _tr);

    int build_req(uint16_t _address, uint16_t _quantity, const uint16_t* _data);

    uint16_t get_req_address() const;
    uint16_t get_req_quantity() const;
    uint16_t get_req_data(uint16_t _offset) const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_quantity() const;
};

// *******************************************************************************
// read_write_regs_t

class read_write_regs_t : public transact_base_t
{
public:
    read_write_regs_t();
    read_write_regs_t(transaction_t& _tr);

    int build_req(uint16_t _rd_address,
                  uint16_t _rd_quantity,
                  uint16_t _wr_address,
                  uint16_t _wr_quantity,
                  const uint16_t* _wr_data);

    uint16_t get_req_rd_address() const;
    uint16_t get_req_rd_quantity() const;
    uint16_t get_req_wr_address() const;
    uint16_t get_req_wr_quantity() const;

    uint16_t get_answer_rd_reg(uint16_t _offset) const;
    uint16_t get_answer_rd_quantity() const;

    void get_answer_rd_regs(uint16_t* _p_data, uint16_t _offset, uint16_t _n_regs);
    void get_answer_rd_regs(regs_t& _res, uint16_t _offset, uint16_t _n_regs);
};

// *******************************************************************************
// write_mask_reg_t

class write_mask_reg_t : public transact_base_t
{
public:
    write_mask_reg_t();
    write_mask_reg_t(transaction_t& _tr);

    int build_req(uint16_t _address,
                  uint16_t _and_mask,
                  uint16_t _or_mask);

    uint16_t get_req_address() const;
    uint16_t get_req_and_mask() const;
    uint16_t get_req_or_mask() const;

    uint16_t get_answer_address() const;
    uint16_t get_answer_and_mask() const;
    uint16_t get_answer_or_mask() const;
};

// *******************************************************************************
// read_file_t

class read_file_t : public transact_base_t
{
public:

    // Read file record for
    struct subreq_t : public emb_read_file_req_t {
        subreq_t(uint16_t _fileno, uint16_t _record_no, uint16_t _length);
    };

    typedef vector<subreq_t> req_t;

    struct subanswer_t {
        uint16_t length;
        emb::regs_t data;
    };

    typedef vector<subanswer_t> answer_t;

    class answer_iterator_t {
        friend class read_file_t;
    private:
        answer_iterator_t(emb_const_pdu_t* _anw);

    public:
        uint16_t subanswer_quantity() const;
        uint16_t subanswer_data(int _i) const;

        bool operator == (const answer_iterator_t& _a) const;
        bool operator != (const answer_iterator_t& _a) const;

        void operator ++ ();
        void operator ++ (int);

        void reset_to_begin();

    private:
        emb_const_pdu_t* ans;
        emb_read_file_subansw_t* iterator_;
    };

    read_file_t();
    read_file_t(transaction_t& _tr);

    int build_req(const req_t& _reqs);

    answer_iterator_t subanswer_begin() const;
    answer_iterator_t subanswer_end() const;

    answer_iterator_t operator [] (int _subanswer_index) const;

    answer_t& get_answer(answer_t& _res);
};

// *******************************************************************************
// write_file_t

class write_file_t : public transact_base_t
{
public:

    struct subreq_t {
        subreq_t(uint16_t _file_number,
                 uint16_t _record_number,
                 const regs_t& _regs);

        uint16_t file_number;
        uint16_t record_number;
        regs_t regs;
    };

    typedef vector<subreq_t> req_t;

    write_file_t();
    write_file_t(transaction_t& _tr);

    int build_req(const req_t& _req);
};

// *******************************************************************************
// read_fifo_t

class read_fifo_t : public transact_base_t
{
public:
    read_fifo_t();
    read_fifo_t(transaction_t& _tr);

    void build_req(uint16_t _starting_address);

    uint16_t get_answer_regs_count() const;
    uint16_t get_answer_data(uint16_t _offset) const;
    uint16_t get_answer_all_data(uint16_t _buf_size, uint16_t* _buf) const;
    regs_t& get_answer_all_data(regs_t& _result) const;
};

// *******************************************************************************
// client_t

class client_t
{
public:
    client_t();

    int do_transaction(int _server_addr,
                       unsigned int _timeout,
                       transaction_t& _transaction);

    void set_transport(struct emb_transport_t* _transport);

    void set_sync(bool _is_sync);
    bool get_sync() const;

    virtual void emb_on_response(int _slave_addr);
    virtual void emb_on_error(int _slave_addr, int _errno);

protected: // (interface for sync mode)
    virtual int emb_sync_client_start_wait(unsigned int _timeout);
    void sync_answer_timeout();

private:
    static void emb_on_response_(struct emb_client_t* _req, int _slave_addr);
    static void emb_on_error_(struct emb_client_t* _req, int _slave_addr, int _errno);

protected:
    struct emb_client_t client;
    transaction_t* curr_transaction;
    int curr_server_addr;
    bool is_sync;
    int result;
};

// *******************************************************************************
// proxy_t

class proxy_t
{
    struct holdings_t
    {
        friend class proxy_t;
    private:
        struct reg_t
        {
            friend class holdings_t;

            operator int() const;
            void operator = (int _v);
            void operator |= (int _v);
            void operator &= (int _v);
            void operator = (const emb::regs_t& _regs);

            operator float() const;
            void operator = (float _v);

            void set_bit(unsigned char _nbit, bool _value);
            bool get_bit(unsigned char _nbit);

        private:
            uint16_t addr;
            proxy_t* p;
        } reg;

        struct regs_t
        {
            friend class holdings_t;

            operator emb::regs_t();
            void operator = (const emb::regs_t& _regs);

        private:
            uint16_t start, end;
            proxy_t* p;
        } regs;

    public:
        reg_t& operator[] (uint16_t _i);
        regs_t& operator[] (const emb::range_t& _r);

    private:
        proxy_t* p;
    };

public:

    enum { DEFAULT_TIMEOUT = 100 };

    proxy_t();

    proxy_t(client_t* _client, int _server_addr);

    void set_client(client_t* _client);
    client_t* client() const;

    void set_server_address(int _server_address);
    int server_address() const;

    void set_timeout(unsigned int _time);
    unsigned int timeout() const;

    // Discrete inputs
    void read_discrete_inputs(uint16_t _begin, uint16_t _size, inputs_t& _result);

    // Coils
    void read_coils(uint16_t _begin, uint16_t _size, coils_t& _result);
    void write_coil(uint16_t _addr, bool _value);
//    void write_coils(uint16_t _begin, const coils_t& _values);

    // Input registers
    void read_input_regs(uint16_t _begin, uint16_t _size, regs_t& _result);

    // Holding registers
    void read_holdings(uint16_t _begin, uint16_t _size, regs_t& _result);
    void write_holding(uint16_t _addr, uint16_t _value);
    void write_holdings(uint16_t _begin, const regs_t& _values);
    void read_write_holdings(uint16_t _read_begin, uint16_t _read_size, regs_t& _read_result,
                             uint16_t _write_begin, const regs_t& _to_write);

    // File records
    void read_file(const read_file_t::req_t& _req, read_file_t::answer_t& _result);
    void write_file(const write_file_t::req_t& _req);

    // FIFO
    void read_fifo(uint16_t _begin, regs_t& _result);

public:
    void do_transaction(transaction_t& _tr);

public:
    holdings_t holdings;

private:
    client_t* client_;
    int server_addr_;
    unsigned int timeout_;
    transaction_t transaction;
};

} // namespace client

namespace server {

// *******************************************************************************
//  #####  #####  ####   #   #  #####  ####
//  #      #      #   #  #   #  #      #   #
//  #####  #####  ####   #   #  #####  ####
//      #  #      #   #   # #   #      #   #
//  #####  #####  #   #    #    #####  #   #

class server_t;

// *******************************************************************************
// coils_t

class coils_t
{
    friend class server_t;
public:
    coils_t();
    coils_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);

    virtual uint8_t on_read_coils(uint16_t _offset,
                                  uint16_t _quantity,
                                  uint8_t* _pvalues);

    virtual uint8_t on_write_coils(uint16_t _offset,
                                   uint16_t _quantity,
                                   const uint8_t* _pvalues);

    uint16_t start() const;
    uint16_t size() const;

private:
    void set_funcs();
    static uint8_t read_coils(struct emb_srv_bits_t* _coils,
                              uint16_t _offset,
                              uint16_t _quantity,
                              uint8_t* _pvalues);
    static uint8_t write_coils(struct emb_srv_bits_t* _coils,
                               uint16_t _offset,
                               uint16_t _quantity,
                               const uint8_t* _pvalues);

    struct emb_srv_bits_t coils;
};

// *******************************************************************************
// discrete_inputs_t

class discrete_inputs_t
{
    friend class server_t;
public:
    discrete_inputs_t();
    discrete_inputs_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);

    virtual uint8_t on_read_bits(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint8_t* _pvalues);

    uint16_t start() const;
    uint16_t size() const;

private:
    void set_funcs();
    static uint8_t read_inputs(struct emb_srv_bits_t* _bits,
                               uint16_t _offset,
                               uint16_t _quantity,
                               uint8_t* _pvalues);

    struct emb_srv_bits_t discrete_inputs;
};

// *******************************************************************************
// input_regs_t

class input_regs_t
{
    friend class server_t;
public:
    input_regs_t();
    input_regs_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);
    virtual uint8_t on_read_regs(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues);

    uint16_t start() const;
    uint16_t size() const;

private:
    void set_funcs();
    static uint8_t read_regs(struct emb_srv_regs_t* _rr,
                             uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues);

    struct emb_srv_regs_t h;
};

// *******************************************************************************
// holding_regs_t

class holding_regs_t
{
    friend class server_t;
public:
    holding_regs_t();
    holding_regs_t(uint16_t _start, uint16_t _size);

    void set_start(uint16_t _start);
    void set_size(uint16_t _size);
    virtual uint8_t on_read_regs(uint16_t _offset,
                                 uint16_t _quantity,
                                 uint16_t* _pvalues);
    virtual uint8_t on_write_regs(uint16_t _offset,
                                  uint16_t _quantity,
                                  const uint16_t* _pvalues);

    uint16_t start() const;
    uint16_t size() const;

private:
    void set_funcs();
    static uint8_t read_regs(struct emb_srv_regs_t* _rr,
                             uint16_t _offset,
                             uint16_t _quantity,
                             uint16_t* _pvalues);
    static uint8_t write_regs(struct emb_srv_regs_t* _rr,
                              uint16_t _offset,
                              uint16_t _quantity,
                              const uint16_t* _pvalues);

    struct emb_srv_regs_t h;
};

// *******************************************************************************
// file_record_t

class file_record_t
{
    friend class server_t;
public:
    file_record_t();
    file_record_t(uint16_t _fileno/*, uint16_t _start, uint16_t _size*/);

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
// super_server_t

class super_server_t;

class server_t
{
    friend class super_server_t;

public:

    enum { MAX_FUNCTION_NUMBER = 255 };

    server_t(int _address);

    virtual int addr() const;

    bool add_function(uint8_t _func_no, emb_srv_function_t _func);

    bool add_coils(coils_t& _coils);
    bool add_discrete_inputs(discrete_inputs_t& _inputs);
    bool add_holding_regs(holding_regs_t& _holdings);
    bool add_input_regs(input_regs_t& _holdings);
    bool add_file(file_record_t& _file);

    virtual uint8_t on_read_fifo(uint16_t _address,
                                 uint16_t* _fifo_buf,
                                 uint8_t* _fifo_count);

private:
    static emb_srv_function_t get_function(struct emb_server_t* _srv, uint8_t _func);

    static struct emb_srv_bits_t* get_coils(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_bits_t* get_discrete_inputs(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_regs_t* get_holding_regs(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_regs_t* get_input_regs(struct emb_server_t* _srv, uint16_t _begin);

    static struct emb_srv_file_t* get_file(struct emb_server_t* _srv, uint16_t _fileno/*, uint16_t _begin*/);

    static uint8_t read_fifo(struct emb_server_t* _srv, uint16_t _address,
                             uint16_t* _fifo_buf, uint8_t* _fifo_count);

    std::vector<emb_srv_function_t> functions;

    typedef std::vector<coils_t*>::iterator coils_iter;
    std::vector<coils_t*> coils;

    typedef std::vector<discrete_inputs_t*>::iterator discr_inputs_iter;
    std::vector<discrete_inputs_t*> discrete_inputs;

    typedef std::vector<holding_regs_t*>::iterator holdnigs_iter;
    std::vector<holding_regs_t*> holdings;

    typedef std::vector<input_regs_t*>::iterator input_regs_iter;
    std::vector<input_regs_t*> input_regs;

    typedef std::vector<file_record_t*>::iterator files_iter;
    std::vector<file_record_t*> files;

protected:
    int address;

private:
    struct emb_server_t srv;
};

// *******************************************************************************
// super_server_t

class super_server_t
{
public:
    super_server_t();
    void set_transport(struct emb_transport_t* _transport);

    bool add_server(server_t& _srv);

private:
    static struct emb_server_t* _get_server(struct emb_super_server_t* _ssrv,
                                            uint8_t _address);
    typedef std::vector<server_t*>::iterator srv_iter;
    std::vector<server_t*> servers;

    struct emb_super_server_t ssrv;
    emb::pdu_t rx_pdu,tx_pdu;
};

} // namespace server

} // namespace emb

#endif // EMODBUS_HPP
