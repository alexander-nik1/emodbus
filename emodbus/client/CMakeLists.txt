
include_directories(../base/)

# ===================================================
# client full

aux_source_directory(. LIB_EMODBUS_CLIENT_FULL_SRC)

add_library(emodbus-client-full SHARED ${LIB_EMODBUS_CLIENT_FULL_SRC})

add_library(emodbus-client-full-static STATIC ${LIB_EMODBUS_CLIENT_FULL_SRC})

# ===================================================
# client

set(EMODBUS_CLIENT_SRC
	client.c
)

add_library(emodbus-client SHARED ${EMODBUS_CLIENT_SRC})

add_library(emodbus-client-static STATIC ${EMODBUS_CLIENT_SRC})

# ===================================================
# client read coils

set(EMODBUS_CLIENT_READ_COILS_SRC
	read_coils.c
)

add_library(emodbus-client-read-coils SHARED ${EMODBUS_CLIENT_READ_COILS_SRC})

add_library(emodbus-client-read-coils-static STATIC ${EMODBUS_CLIENT_READ_COILS_SRC})

# ===================================================
# client write coil

set(EMODBUS_CLIENT_WRITE_COIL_SRC
	write_coil.c
)

add_library(emodbus-client-write-coil SHARED ${EMODBUS_CLIENT_WRITE_COIL_SRC})

add_library(emodbus-client-write-coil-static STATIC ${EMODBUS_CLIENT_WRITE_COIL_SRC})

# ===================================================
# client write coils

set(EMODBUS_CLIENT_WRITE_COILS_SRC
	write_coils.c
)

add_library(emodbus-client-write-coils SHARED ${EMODBUS_CLIENT_WRITE_COILS_SRC})

add_library(emodbus-client-write-coils-static STATIC ${EMODBUS_CLIENT_WRITE_COILS_SRC})

# ===================================================
# client read regs

set(EMODBUS_CLIENT_READ_REGS_SRC
	read_regs.c
)

add_library(emodbus-client-read-regs SHARED ${EMODBUS_CLIENT_READ_REGS_SRC})

add_library(emodbus-client-read-regs-static STATIC ${EMODBUS_CLIENT_READ_REGS_SRC})

# ===================================================
# client mask write reg

set(EMODBUS_CLIENT_MASK_WRITE_REG
	write_mask_reg.c
)

add_library(emodbus-client-mask-write-reg SHARED ${EMODBUS_CLIENT_MASK_WRITE_REG})

add_library(emodbus-client-mask-write-reg-static STATIC ${EMODBUS_CLIENT_MASK_WRITE_REG})

# ===================================================
# client write single register

set(EMODBUS_CLIENT_WRITE_SINGLE_REG
	write_single_reg.c
)

add_library(emodbus-client-write-single-reg SHARED ${EMODBUS_CLIENT_WRITE_SINGLE_REG})

add_library(emodbus-client-write-single-reg-static STATIC ${EMODBUS_CLIENT_WRITE_SINGLE_REG})

# ===================================================
# client write multiple registers

set(EMODBUS_CLIENT_WRITE_MULTI_REGS
	write_multi_regs.c
)

add_library(emodbus-client-write-multi-reg SHARED ${EMODBUS_CLIENT_WRITE_MULTI_REGS})

add_library(emodbus-client-write-multi-reg-static STATIC ${EMODBUS_CLIENT_WRITE_MULTI_REGS})

# ===================================================
# client read file record

set(EMODBUS_CLIENT_READ_FILE_RECORD
	read_file_record.c
)

add_library(emodbus-client-read-file-record SHARED ${EMODBUS_CLIENT_READ_FILE_RECORD})

add_library(emodbus-client-read-file-record-static STATIC ${EMODBUS_CLIENT_READ_FILE_RECORD})

# ===================================================
# client write file record

set(EMODBUS_CLIENT_WRITE_FILE_RECORD
	write_file_record.c
)

add_library(emodbus-client-write-file-record SHARED ${EMODBUS_CLIENT_WRITE_FILE_RECORD})

add_library(emodbus-client-write-file-record-static STATIC ${EMODBUS_CLIENT_WRITE_FILE_RECORD})

# ===================================================
# client read FIFO

set(EMODBUS_CLIENT_READ_FIFO
	read_fifo.c
)

add_library(emodbus-client-read-fifo SHARED ${EMODBUS_CLIENT_READ_FIFO})

add_library(emodbus-client-read-fifo-static STATIC ${EMODBUS_CLIENT_READ_FIFO})
