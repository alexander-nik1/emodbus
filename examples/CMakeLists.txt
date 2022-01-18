
add_executable(ascii-rtu-server ascii-rtu-server.c)
target_link_libraries(ascii-rtu-server emodbus)

add_executable(ascii-rtu-client ascii-rtu-client.c)
target_link_libraries(ascii-rtu-client emodbus)
