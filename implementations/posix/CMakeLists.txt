
include_directories(../include)

add_library(emb-tcp-client STATIC tcp-client.c)

add_library(emb-tcp-server STATIC tcp-server.c)

add_library(emb-serail-port STATIC serial-port.c)

add_library(emodbus-impl-posix SHARED
    mb-rtu-via-serial.c
    mb-rtu-via-tcp-client.c
#    mb-rtu-via-tcp-server.c
    mb-tcp-via-tcp-client.c
    mb-tcp-via-tcp-server.c
    client.c
    dumper.c
    pdu.c
)

target_link_libraries(emodbus-impl-posix
    pthread
    event_core
    emodbus-transport
    emodbus-client
    emodbus-server
    emb-tcp-client
    emb-tcp-server
    emb-serail-port
)

add_library(emodbus-cpp-impl-posix SHARED
    client.cpp
)

target_link_libraries(emodbus-cpp-impl-posix
    emodbus-cpp
    emodbus-impl-posix
)
