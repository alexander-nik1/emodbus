
add_library(emodbus-full-cpp emodbus.cpp)

target_link_libraries(emodbus-full-cpp
    emodbus-base-static
    emodbus-client-full-static
    emodbus-server-static
)
