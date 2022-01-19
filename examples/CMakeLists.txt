
add_executable(ascii-tty-server ascii-tty-server.c)
target_link_libraries(ascii-tty-server emodbus)

add_executable(ascii-tty-client ascii-tty-client.c)
target_link_libraries(ascii-tty-client emodbus)

add_executable(rtu-tty-server rtu-tty-server.c)
target_link_libraries(rtu-tty-server emodbus)

add_executable(rtu-tty-client rtu-tty-client.c)
target_link_libraries(rtu-tty-client emodbus)

add_executable(tcp-tcp-server tcp-tcp-server.c)
target_link_libraries(tcp-tcp-server emodbus)
