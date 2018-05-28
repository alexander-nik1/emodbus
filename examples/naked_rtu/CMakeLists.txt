
add_executable(rtu_simple_server rtu_via_tty.c server.c)

target_link_libraries(rtu_simple_server
    emodbus-base
    emodbus-server
    emodbus-transport
)
