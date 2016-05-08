
add_library(stream-posix-serial SHARED
    stream-posix-serial.h
    stream-posix-serial.c
)

add_library(rtu SHARED
    rtu.cpp
    rtu.hpp
)

target_link_libraries(rtu
    stream-posix-serial
    #stream-tcp-client
    emodbus-rtu
    event
)
