
include_directories(../base/)


aux_source_directory(. LIB_EMODBUS_CLIENT_FULL_SRC)

add_library(emodbus-client-full SHARED ${LIB_EMODBUS_CLIENT_FULL_SRC})
target_link_libraries(emodbus-client-full emodbus-base)

# ===================================================
# client base

set(EMODBUS_CLIENT_BASE_SRC
	client_base.h
	client_base.c
)

add_library(emodbus-client-base SHARED ${EMODBUS_CLIENT_BASE_SRC})

add_library(emodbus-client-base-static STATIC ${EMODBUS_CLIENT_BASE_SRC})

target_link_libraries(emodbus-client-base emodbus-base)

# ===================================================
# client scheduler 

set(EMODBUS_CLIENT_SCHED_SRC
	modbus_sched.h
	modbus_sched.c
)

add_library(emodbus-client-sched SHARED ${EMODBUS_CLIENT_SCHED_SRC})

add_library(emodbus-client-sched-static STATIC ${EMODBUS_CLIENT_SCHED_SRC})

# ===================================================
# client read holding regs

set(EMODBUS_CLIENT_READ_HOLDING_REGS_SRC
	read_holding_regs.h
	read_holding_regs.c
)

add_library(emodbus-client-read-holdings SHARED ${EMODBUS_CLIENT_READ_HOLDING_REGS_SRC})

add_library(emodbus-client-read-holdings-static STATIC ${EMODBUS_CLIENT_READ_HOLDING_REGS_SRC})

# ===================================================
# client mask write reg

set(EMODBUS_CLIENT_MASK_WRITE_REG
	write_mask_reg.h
	write_mask_reg.c
)

add_library(emodbus-client-mask-write-reg SHARED ${EMODBUS_CLIENT_MASK_WRITE_REG})

add_library(emodbus-client-mask-write-reg-static STATIC ${EMODBUS_CLIENT_MASK_WRITE_REG})

# ===================================================
# client write multiple registers

set(EMODBUS_CLIENT_WRITE_MULTI_REGS
	write_multi_regs.h
	write_multi_regs.c
)

add_library(emodbus-client-write-multi-reg SHARED ${EMODBUS_CLIENT_WRITE_MULTI_REGS})

add_library(emodbus-client-write-multi-reg-static STATIC ${EMODBUS_CLIENT_WRITE_MULTI_REGS})
