

add_library(posix_serial_rtu SHARED
    posix-serial-port.h
    posix-serial-port.c
    posix_serial_rtu.cpp
    posix_serial_rtu.hpp
)

target_link_libraries(posix_serial_rtu
    emodbus-rtu
    event
)
