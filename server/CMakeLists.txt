
set(EMODBUS_SERVER_SRC
    server.c
)

add_library(emodbus-server SHARED ${EMODBUS_SERVER_SRC})
add_library(emodbus-server-static STATIC ${EMODBUS_SERVER_SRC})
