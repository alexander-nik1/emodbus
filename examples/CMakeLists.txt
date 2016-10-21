
include_directories(../implementations/include)

add_executable(server-example server.c)

add_executable(client-example client.c)

target_link_libraries(client-example
    event_pthreads
    pthread
    emodbus-base
    emodbus-client-full
    emodbus-impl-posix
)
