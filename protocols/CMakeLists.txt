
set(LIB_EMODBUS_RTU
	rtu.h
	rtu.c
)

add_library(emodbus-rtu SHARED ${LIB_EMODBUS_RTU})
target_link_libraries(emodbus-rtu emodbus-base)

add_library(emodbus-rtu-static STATIC ${LIB_EMODBUS_RTU})

