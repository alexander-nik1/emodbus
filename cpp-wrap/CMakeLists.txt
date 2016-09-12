
add_library(emodbus-cpp SHARED emodbus.cpp)

target_link_libraries(emodbus-cpp
    emodbus-client-full
    emodbus-server
    emodbus-base
)
