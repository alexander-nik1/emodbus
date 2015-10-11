
set(EMODBUS_SERVER_BASE_SRC
	server_base.h
	server_base.c
)

add_library(emodbus-server-base SHARED ${EMODBUS_SERVER_BASE_SRC})
add_library(emodbus-server-base-static STATIC ${EMODBUS_SERVER_BASE_SRC})
