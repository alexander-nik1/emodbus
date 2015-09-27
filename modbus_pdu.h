
#ifndef MODBUS_PROTOCOL_DATA_UNIT_H
#define MODBUS_PROTOCOL_DATA_UNIT_H

struct modbus_pdu_t {
    int function;
    int data_size;
    void* data;
};

struct modbus_const_pdu_t {
    int function;
    int data_size;
    const void* data;
};

#define MB_CONST_PDU(_pdu_) ((struct modbus_const_pdu_t*)(_pdu_))

#endif // MODBUS_PROTOCOL_DATA_UNIT_H
