
#ifndef EMODBUS_RTU_VIA_SERIAL_H
#define EMODBUS_RTU_VIA_SERIAL_H

#include <emodbus/base/modbus_transport.h>
#include <event2/event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct emb_rtu_via_serial_t;

struct emb_rtu_via_serial_t*
emb_rtu_via_serial_create(struct event_base *_base,
                          unsigned int _timeout_ms,
                          const char* _dev_name,
                          unsigned int _baudrate);

void emb_rtu_via_serial_destroy(struct emb_rtu_via_serial_t* _ctx);

void emb_rtu_via_serial_set_cb(struct emb_rtu_via_serial_t* _ctx,
                               emb_on_rx_pdu_t _on_rx,
                               emb_on_error_t _on_err,
                               void* _context);

void emb_rtu_via_serial_send(struct emb_rtu_via_serial_t* _ctx,
                             const struct emb_transport_info_t* _info);

#ifdef __cplusplus
};
#endif

#endif // EMODBUS_RTU_VIA_SERIAL_H
