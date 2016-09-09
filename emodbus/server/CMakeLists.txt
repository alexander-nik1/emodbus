
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

###############################################################################################################
# Coils

# ===================================================
# server-read-coils

set(EMODBUS_SERVER_READ_COILS
    read_coils.c
)

add_library(emodbus-server-read-coils SHARED ${EMODBUS_SERVER_READ_COILS})
add_library(emodbus-server-read-coils-static STATIC ${EMODBUS_SERVER_READ_COILS})

# ===================================================
# server-write-coil

set(EMODBUS_SERVER_WRITE_COIL
    write_coil.c
)

add_library(emodbus-server-write-coil SHARED ${EMODBUS_SERVER_WRITE_COIL})
add_library(emodbus-server-write-coil-static STATIC ${EMODBUS_SERVER_WRITE_COIL})


# ===================================================
# server-write-coils

set(EMODBUS_SERVER_WRITE_COILS
    write_coils.c
)

add_library(emodbus-server-write-coils SHARED ${EMODBUS_SERVER_WRITE_COILS})
add_library(emodbus-server-write-coils-static STATIC ${EMODBUS_SERVER_WRITE_COILS})

###############################################################################################################
# Holdings

# ===================================================
# server-read-holdings

set(EMODBUS_SERVER_READ_HOLDINGS
    read_holding_regs.c
)

add_library(emodbus-server-read-holdings SHARED ${EMODBUS_SERVER_READ_HOLDINGS})
add_library(emodbus-server-read-holdings-static STATIC ${EMODBUS_SERVER_READ_HOLDINGS})

# ===================================================
# server-write-reg

set(EMODBUS_SERVER_WRITE_REG
    write_single_reg.c
)

add_library(emodbus-server-write-reg SHARED ${EMODBUS_SERVER_WRITE_REG})
add_library(emodbus-server-write-reg-static STATIC ${EMODBUS_SERVER_WRITE_REG})

# ===================================================
# server-write-regs

set(EMODBUS_SERVER_WRITE_REGS
    write_multi_regs.c
)

add_library(emodbus-server-write-regs SHARED ${EMODBUS_SERVER_WRITE_REGS})
add_library(emodbus-server-write-regs-static STATIC ${EMODBUS_SERVER_WRITE_REGS})

# ===================================================
# server-mask-reg

set(EMODBUS_SERVER_MASK_REG
    write_mask_reg.c
)

add_library(emodbus-server-mask-reg SHARED ${EMODBUS_SERVER_MASK_REG})
add_library(emodbus-server-mask-reg-static STATIC ${EMODBUS_SERVER_MASK_REG})

###############################################################################################################
# Files

# ===================================================
# server-read-file

set(EMODBUS_SERVER_READ_FILE
    read_file.c
)

add_library(emodbus-server-read-file SHARED ${EMODBUS_SERVER_READ_FILE})
add_library(emodbus-server-read-file-static STATIC ${EMODBUS_SERVER_READ_FILE})

# ===================================================
# server-write-file

set(EMODBUS_SERVER_WRITE_FILE
    write_file.c
)

add_library(emodbus-server-write-file SHARED ${EMODBUS_SERVER_WRITE_FILE})
add_library(emodbus-server-write-file-static STATIC ${EMODBUS_SERVER_WRITE_FILE})
