
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

struct emb_transport_t*
emb_rtu_via_serial_get_transport(struct emb_rtu_via_serial_t* _ctx);

#ifdef __cplusplus
};
#endif

#endif // EMODBUS_RTU_VIA_SERIAL_H
