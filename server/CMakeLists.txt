
# ===================================================
# server-full

aux_source_directory(. LIB_EMODBUS_SERVER_FULL_SRC)

add_library(emodbus-server-full SHARED ${LIB_EMODBUS_SERVER_FULL_SRC})
add_library(emodbus-server-full-static STATIC ${LIB_EMODBUS_SERVER_FULL_SRC})

# ===================================================
# server

set(EMODBUS_SERVER_SRC
    server.c
    exception.c
)

add_library(emodbus-server SHARED ${EMODBUS_SERVER_SRC})
add_library(emodbus-server-static STATIC ${EMODBUS_SERVER_SRC})

# ===================================================
# server-read-holdings

set(EMODBUS_SERVER_READ_HOLDINGS
    read_holding_regs.c
)

add_library(emodbus-server-read-holdings SHARED ${EMODBUS_SERVER_READ_HOLDINGS})
add_library(emodbus-server-read-holdings-static STATIC ${EMODBUS_SERVER_READ_HOLDINGS})
