
add_library(emodbus-full-cpp emodbus.cpp)

target_link_libraries(emodbus-full-cpp
    emodbus-client-full
    emodbus-server
    emodbus-base
)
