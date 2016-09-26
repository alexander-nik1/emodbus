
set(LIB_EMODBUS_RTU rtu.c)

add_library(emodbus-rtu SHARED ${LIB_EMODBUS_RTU})
target_link_libraries(emodbus-rtu emodbus-base)

add_library(emodbus-rtu-static STATIC ${LIB_EMODBUS_RTU})


set(LIB_EMODBUS_TCP tcp.c)

add_library(emodbus-tcp SHARED ${LIB_EMODBUS_TCP})
target_link_libraries(emodbus-tcp emodbus-base)

add_library(emodbus-tcp-static STATIC ${LIB_EMODBUS_TCP})
