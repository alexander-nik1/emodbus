
add_library(emodbus-full-cpp SHARED emodbus.cpp)

target_link_libraries(emodbus-full-cpp
    emodbus-client-full
    emodbus-server
    emodbus-base
)
