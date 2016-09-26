
set(LIB_EMODBUS_BASE_SRC
    add/crc.c
    add/list.c
    add/s-list.c
    common.c
    modbus_errno.c
    modbus_pdu.c
    modbus_transport.c
)

add_library(emodbus-base SHARED ${LIB_EMODBUS_BASE_SRC})
add_library(emodbus-base-static STATIC ${LIB_EMODBUS_BASE_SRC})
