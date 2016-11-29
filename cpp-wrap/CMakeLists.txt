
add_library(emodbus-cpp SHARED emodbus.cpp)

target_link_libraries(emodbus-cpp
    emodbus-client
    emodbus-server
    emodbus-base
)
