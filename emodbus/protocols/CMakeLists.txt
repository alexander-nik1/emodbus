
aux_source_directory(. LIB_EMODBUS_TRANSPORT)

aux_source_directory(add LIB_EMODBUS_TRANSPORT_ADD)

add_library(emodbus-transport SHARED
    ${LIB_EMODBUS_TRANSPORT}
    ${LIB_EMODBUS_TRANSPORT_ADD}
)

target_link_libraries(emodbus-transport emodbus-base)

set(LIB_EMODBUS_RTU rtu.c add/crc.c)
set(LIB_EMODBUS_TCP tcp.c)
set(LIB_EMODBUS_ASCII ascii.c)

add_library(emodbus-rtu SHARED ${LIB_EMODBUS_RTU})
add_library(emodbus-tcp SHARED ${LIB_EMODBUS_TCP})
add_library(emodbus-ascii SHARED ${LIB_EMODBUS_ASCII})

target_link_libraries(emodbus-rtu emodbus-base)
target_link_libraries(emodbus-tcp emodbus-base)
target_link_libraries(emodbus-ascii emodbus-base)

add_library(emodbus-tcp-static STATIC ${LIB_EMODBUS_TCP})
add_library(emodbus-rtu-static STATIC ${LIB_EMODBUS_RTU})
add_library(emodbus-ascii-static STATIC ${LIB_EMODBUS_ASCII})
