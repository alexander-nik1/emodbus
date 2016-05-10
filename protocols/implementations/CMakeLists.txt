
add_library(emodbus-serial-rtu SHARED
    serial-rtu.cpp
)

target_link_libraries(emodbus-serial-rtu emodbus-rtu event_core)

add_library(emodbus-tcp-client-rtu SHARED
    tcp-client-rtu.cpp
)

target_link_libraries(emodbus-tcp-client-rtu emodbus-rtu event_core)
