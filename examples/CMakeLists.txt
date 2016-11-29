
include_directories(../implementations/include)

add_executable(server-example server.c)

add_executable(client-example client.c)

add_executable(client-cpp-example client.cpp)

target_link_libraries(client-example
    event_pthreads
    pthread
    emodbus-base
    emodbus-client
    emodbus-impl-posix
)

target_link_libraries(server-example
    event_pthreads
    pthread
    emodbus-base
    emodbus-server
    emodbus-impl-posix
)

target_link_libraries(client-cpp-example
    event_pthreads
    pthread
    emodbus-base
    emodbus-server
    emodbus-cpp-impl-posix
)

