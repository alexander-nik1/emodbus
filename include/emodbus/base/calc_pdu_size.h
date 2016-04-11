
#ifndef EMODBUS_CALC_PDU_SIZE_H
#define EMODBUS_CALC_PDU_SIZE_H

#define READ_HOLDINGS_REQ_SIZE()                        (4)
#define READ_HOLDINGS_ANS_SIZE(_quantity_)              (1+(_quantity_)*2)

#define MASK_REGISTER_REQ_SIZE()                        (6)
#define MASK_REGISTER_ANS_SIZE()                        (6)

#define WRITE_REGISTERS_REQ_SIZE(_quantity_)            (5 + (_quantity_) * 2)
#define WRITE_REGISTERS_ANS_SIZE()                      (4)

#define WRITE_REGISTER_REQ_SIZE()                       (4)
#define WRITE_REGISTER_ANS_SIZE()                       (4)

#define READ_FILE_REQ_SIZE(_n_subreqs_)                 (1 + 7 * (_n_subreqs_))
//#define READ_FILE_ANS_SIZE(_subreqs_, _n_subreqs_)

//#define WRITE_FILE_REQ_SIZE()
//#define WRITE_FILE_ANS_SIZE()

#define READ_FIFO_REQ_SIZE()                            (2)
#define READ_FIFO_ANS_SIZE()                            (2 + 2 + 2 * 31)

#endif // EMODBUS_CALC_PDU_SIZE_H
