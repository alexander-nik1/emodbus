

SET(SYNC_CLI_TEST_SRC
    posix-serial-port.h
    posix-serial-port.c
    sync_client_test.cpp
    timespec_operations.h
    timespec_operations.c
)

SET(CXXSTANDARD "-std=c++11")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXSTANDARD}")


add_executable(sync_client_test ${SYNC_CLI_TEST_SRC})

target_link_libraries(sync_client_test
    emodbus-base
    emodbus-client
    emodbus-client-read-holdings
    emodbus-client-write-multi-reg
    emodbus-rtu
    emodbus-full-cpp
    event
    pthread
)
