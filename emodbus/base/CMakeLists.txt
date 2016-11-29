
set(LIB_EMODBUS_BASE_SRC
    add/list.c
    add/s-list.c
    common.c
    modbus_errno.c
    modbus_pdu.c
    modbus_transport.c
)

add_library(emodbus-base SHARED ${LIB_EMODBUS_BASE_SRC})

if(DEFINED EMB_BUILD_STATIC_LIBS)
add_library(emodbus-base-static STATIC ${LIB_EMODBUS_BASE_SRC})
endif()
