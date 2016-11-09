
#ifndef EMB_DUMPING_HELPER_H
#define EMB_DUMPING_HELPER_H

class emb_debug_helper_t {
public:

    emb_debug_helper_t();

    void enable_dumping();
    void disable_dumping();
    void enable_rx_dumping();
    void disable_rx_dumping();
    void enable_tx_dumping();
    void disable_tx_dumping();

    unsigned long rx_bytes() const;
    unsigned long tx_bytes() const;

private:
    static void dbg_print_packet(void* _f, const char* _prefix, const void* _pkt, unsigned int _size);

    static void on_write_rx(const void* _data, unsigned int _size);
    static void on_write_tx(const void* _data, unsigned int _size);

    bool rx_dumping;
    bool tx_dumping;

    unsigned long rx_bytes_;
    unsigned long tx_bytes_;
};

extern emb_debug_helper_t emb_debug_helper;

#endif // EMB_DUMPING_HELPER_H
