
SET(SYNC_CLI_TEST_SRC
    sync_client_test.cpp
    timespec_operations.h
    timespec_operations.c
    dumping_helper.hpp
    dumping_helper.cpp
)

SET(SRV_TEST_SRC
    server_test.cpp
    dumping_helper.hpp
    dumping_helper.cpp
)

#SET(CXXSTANDARD "-std=c++11")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXSTANDARD}")

#add_subdirectory(posix_serial_rtu)
#add_subdirectory(rtu)

include_directories(../implementations/include)

add_executable(sync_client_test ${SYNC_CLI_TEST_SRC})

add_executable(srv_test ${SRV_TEST_SRC})

target_link_libraries(srv_test
    emodbus-base
    emodbus-server-full
    emodbus-full-cpp
    pthread
    emodbus-serial-rtu
    emodbus-tcp-client-rtu
    emodbus-impl-posix
)


target_link_libraries(sync_client_test
    emodbus-base
    emodbus-client-full
    emodbus-full-cpp
    pthread
    emodbus-serial-rtu
    emodbus-impl-posix
    event_pthreads
)

add_executable(ev_srv event_server.c)

target_link_libraries(ev_srv event_core)
