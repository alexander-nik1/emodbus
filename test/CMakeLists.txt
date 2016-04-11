

SET(SYNC_CLI_TEST_SRC
    posix-serial-port.h
    posix-serial-port.c
    sync_client_test.cpp
    timespec_operations.h
    timespec_operations.c
)

SET(SRV_TEST_SRC
    posix-serial-port.h
    posix-serial-port.c
    server_test.cpp
    timespec_operations.h
    timespec_operations.c
)


SET(CXXSTANDARD "-std=c++11")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXSTANDARD}")


add_executable(sync_client_test ${SYNC_CLI_TEST_SRC})

add_executable(srv_test ${SRV_TEST_SRC})

target_link_libraries(srv_test
    emodbus-base
    emodbus-server-full
    emodbus-rtu
    emodbus-full-cpp
    event
    pthread
)


target_link_libraries(sync_client_test
    emodbus-base
    emodbus-client-full
    emodbus-rtu
    emodbus-full-cpp
    event
    pthread
)
