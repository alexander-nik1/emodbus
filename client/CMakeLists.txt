
include_directories(../base/)

# ===================================================
# client full

aux_source_directory(. LIB_EMODBUS_CLIENT_FULL_SRC)

add_library(emodbus-client-full SHARED ${LIB_EMODBUS_CLIENT_FULL_SRC})
target_link_libraries(emodbus-client-full emodbus-base)

# ===================================================
# sync client

set(EMODBUS_SYNC_CLIENT_SRC
	sync_client.c
)

add_library(emodbus-sync-client SHARED ${EMODBUS_SYNC_CLIENT_SRC})

add_library(emodbus-sync-client-static STATIC ${EMODBUS_SYNC_CLIENT_SRC})

target_link_libraries(emodbus-sync-client emodbus-base)


# ===================================================
# client

set(EMODBUS_CLIENT_SRC
	client.c
)

add_library(emodbus-client SHARED ${EMODBUS_CLIENT_SRC})

add_library(emodbus-client-static STATIC ${EMODBUS_CLIENT_SRC})


# ===================================================
# async client

set(EMODBUS_ASYNC_CLIENT_SRC
	async_client.c
)

add_library(emodbus-async-client SHARED ${EMODBUS_ASYNC_CLIENT_SRC})

add_library(emodbus-async-client-static STATIC ${EMODBUS_ASYNC_CLIENT_SRC})

# ===================================================
# client read holding regs

set(EMODBUS_CLIENT_READ_HOLDING_REGS_SRC
	read_holding_regs.c
)

add_library(emodbus-client-read-holdings SHARED ${EMODBUS_CLIENT_READ_HOLDING_REGS_SRC})

add_library(emodbus-client-read-holdings-static STATIC ${EMODBUS_CLIENT_READ_HOLDING_REGS_SRC})

# ===================================================
# client mask write reg

set(EMODBUS_CLIENT_MASK_WRITE_REG
	write_mask_reg.c
)

add_library(emodbus-client-mask-write-reg SHARED ${EMODBUS_CLIENT_MASK_WRITE_REG})

add_library(emodbus-client-mask-write-reg-static STATIC ${EMODBUS_CLIENT_MASK_WRITE_REG})

# ===================================================
# client write multiple registers

set(EMODBUS_CLIENT_WRITE_MULTI_REGS
	write_multi_regs.c
)

add_library(emodbus-client-write-multi-reg SHARED ${EMODBUS_CLIENT_WRITE_MULTI_REGS})

add_library(emodbus-client-write-multi-reg-static STATIC ${EMODBUS_CLIENT_WRITE_MULTI_REGS})
