
set(LIB_EMODBUS_BASE_SRC
    add/container_of.h
    add/crc.h
    add/crc.c
    add/list.h
    add/list.c
    add/scheduler.h
    add/scheduler.c
    add/stream.h
    add/stream.c
    byte-word.h
    common.c
    common.h
    modbus_errno.h
    modbus_errno.c
    modbus_pdu.h
    modbus_proto.c
    modbus_proto.h
)

add_library(emodbus-base SHARED ${LIB_EMODBUS_BASE_SRC})
add_library(emodbus-base-static STATIC ${LIB_EMODBUS_BASE_SRC})
