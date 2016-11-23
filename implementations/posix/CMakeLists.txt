
include_directories(../include)

add_library(tcp-client STATIC tcp-client.c)

if( NOT DEFINED EMB_DO_NOT_BUILD_TCP_SERVER )
add_library(tcp-server STATIC tcp-server.c)
set ( MY_TCP_SRV tcp-server )
endif()

add_library(serail-port STATIC serial-port.c)

add_library(emodbus-serial-rtu SHARED
    serial-rtu.cpp
)

target_link_libraries(emodbus-serial-rtu emodbus-rtu event)

add_library(emodbus-tcp-client-rtu SHARED
    tcp-client-rtu.cpp
)

target_link_libraries(emodbus-tcp-client-rtu emodbus-rtu event tcp-client)

add_library(emodbus-impl-posix SHARED
    mb-rtu-via-serial.c
#    mb-rtu-via-tcp-client.c
#    mb-rtu-via-tcp-server.c
    mb-tcp-via-tcp-client.c
    mb-tcp-via-tcp-server.c
    client.c
    dumper.c
    pdu.c
)

target_link_libraries(emodbus-impl-posix
    pthread
    event
    emodbus-rtu
    emodbus-tcp
    emodbus-client
    tcp-client
	${MY_TCP_SRV}
    serail-port
)

add_library(emodbus-cpp-impl-posix SHARED
    client.cpp
)

target_link_libraries(emodbus-cpp-impl-posix
    emodbus-cpp
    emodbus-impl-posix
)
