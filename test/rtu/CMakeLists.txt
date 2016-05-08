
add_library(emodbus-serial-rtu SHARED
    serial-rtu.cpp
    serial-rtu.hpp
)

target_link_libraries(emodbus-serial-rtu emodbus-rtu event)

add_library(emodbus-tcp-client-rtu SHARED
    tcp-client-rtu.cpp
    tcp-client-rtu.hpp
)

target_link_libraries(emodbus-tcp-client-rtu emodbus-rtu event)
