
add_library(emodbus-serial-rtu SHARED
    serial-rtu.cpp
    serial-rtu.hpp
)

target_link_libraries(emodbus-serial-rtu
    emodbus-rtu
    event
)
