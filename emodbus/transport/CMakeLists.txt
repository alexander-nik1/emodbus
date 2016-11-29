
aux_source_directory(. LIB_EMODBUS_TRANSPORT)

aux_source_directory(add LIB_EMODBUS_TRANSPORT_ADD)

add_library(emodbus-transport SHARED
    ${LIB_EMODBUS_TRANSPORT}
    ${LIB_EMODBUS_TRANSPORT_ADD}
)

target_link_libraries(emodbus-transport emodbus-base)

if(DEFINED EMB_BUILD_PARTIAL_LIBRARIES)

set(LIB_EMODBUS_RTU rtu.c add/crc.c)
set(LIB_EMODBUS_TCP tcp.c)

add_library(emodbus-rtu SHARED ${LIB_EMODBUS_RTU})
add_library(emodbus-tcp SHARED ${LIB_EMODBUS_TCP})

target_link_libraries(emodbus-rtu emodbus-base)
target_link_libraries(emodbus-tcp emodbus-base)

if(DEFINED EMB_BUILD_STATIC_LIBS)
add_library(emodbus-tcp-static STATIC ${LIB_EMODBUS_TCP})
add_library(emodbus-rtu-static STATIC ${LIB_EMODBUS_RTU})
endif(DEFINED EMB_BUILD_STATIC_LIBS)

endif(DEFINED EMB_BUILD_PARTIAL_LIBRARIES)
