
set(LIB_EMODBUS_BASE_SRC
    add/crc.c
    add/list.c
    add/stream.c
    common.c
    modbus_errno.c
    modbus_pdu.c
    modbus_proto.c
)

add_library(emodbus-base SHARED ${LIB_EMODBUS_BASE_SRC})
add_library(emodbus-base-static STATIC ${LIB_EMODBUS_BASE_SRC})
