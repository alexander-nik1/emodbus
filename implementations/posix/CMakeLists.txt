
add_library(tcp-client STATIC tcp-client.c)

add_library(tcp-server STATIC tcp-server.c)

add_library(emodbus-serial-rtu SHARED
    serial-rtu.cpp
)

target_link_libraries(emodbus-serial-rtu emodbus-rtu event_core)

add_library(emodbus-tcp-client-rtu SHARED
    tcp-client-rtu.cpp
)

target_link_libraries(emodbus-tcp-client-rtu emodbus-rtu event_core tcp-client)

add_library(emodbus-tcp-client-tcp SHARED
    mb-tcp-via-tcp-client.c
)

target_link_libraries(emodbus-tcp-client-tcp emodbus-tcp event_core tcp-client)


add_library(emodbus-tcp-server-tcp SHARED
    mb-tcp-via-tcp-server.c
)

target_link_libraries(emodbus-tcp-server-tcp emodbus-tcp event_core tcp-server)
