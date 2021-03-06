
include_directories(../base/)

aux_source_directory(. LIB_EMODBUS_CLIENT_SRC)

add_library(emodbus-client SHARED ${LIB_EMODBUS_CLIENT_SRC})

if(DEFINED EMB_BUILD_STATIC_LIBS)
add_library(emodbus-client-static STATIC ${LIB_EMODBUS_CLIENT_SRC})
endif()

if(DEFINED EMB_BUILD_PARTIAL_LIBRARIES)

set(EMODBUS_CLIENT_SRC client.c)
set(EMODBUS_CLIENT_READ_BITS_SRC read_bits.c)
set(EMODBUS_CLIENT_WRITE_COIL_SRC write_coil.c)
set(EMODBUS_CLIENT_WRITE_COILS_SRC write_coils.c)
set(EMODBUS_CLIENT_READ_REGS_SRC read_regs.c)
set(EMODBUS_CLIENT_MASK_WRITE_REG write_mask_reg.c)
set(EMODBUS_CLIENT_WRITE_SINGLE_REG write_single_reg.c)
set(EMODBUS_CLIENT_WRITE_MULTI_REGS write_multi_regs.c)
set(EMODBUS_CLIENT_READ_WRITE_REGS read_write_regs.c)
set(EMODBUS_CLIENT_READ_FILE_RECORD read_file_record.c)
set(EMODBUS_CLIENT_WRITE_FILE_RECORD write_file_record.c)
set(EMODBUS_CLIENT_READ_FIFO read_fifo.c)

add_library(emodbus-client SHARED ${EMODBUS_CLIENT_SRC})
add_library(emodbus-client-read-bits SHARED ${EMODBUS_CLIENT_READ_BITS_SRC})
add_library(emodbus-client-write-coil SHARED ${EMODBUS_CLIENT_WRITE_COIL_SRC})
add_library(emodbus-client-write-coils SHARED ${EMODBUS_CLIENT_WRITE_COILS_SRC})
add_library(emodbus-client-read-regs SHARED ${EMODBUS_CLIENT_READ_REGS_SRC})
add_library(emodbus-client-mask-write-reg SHARED ${EMODBUS_CLIENT_MASK_WRITE_REG})
add_library(emodbus-client-write-multi-reg SHARED ${EMODBUS_CLIENT_WRITE_MULTI_REGS})
add_library(emodbus-client-read-write-regs SHARED ${EMODBUS_CLIENT_READ_WRITE_REGS})
add_library(emodbus-client-read-file-record SHARED ${EMODBUS_CLIENT_READ_FILE_RECORD})
add_library(emodbus-client-write-file-record SHARED ${EMODBUS_CLIENT_WRITE_FILE_RECORD})
add_library(emodbus-client-read-fifo SHARED ${EMODBUS_CLIENT_READ_FIFO})


if(DEFINED EMB_BUILD_STATIC_LIBS)
add_library(emodbus-client-static STATIC ${EMODBUS_CLIENT_SRC})
add_library(emodbus-client-read-bits-static STATIC ${EMODBUS_CLIENT_READ_BITS_SRC})
add_library(emodbus-client-write-coil-static STATIC ${EMODBUS_CLIENT_WRITE_COIL_SRC})
add_library(emodbus-client-write-coils-static STATIC ${EMODBUS_CLIENT_WRITE_COILS_SRC})
add_library(emodbus-client-read-regs-static STATIC ${EMODBUS_CLIENT_READ_REGS_SRC})
add_library(emodbus-client-mask-write-reg-static STATIC ${EMODBUS_CLIENT_MASK_WRITE_REG})
add_library(emodbus-client-write-single-reg-static STATIC ${EMODBUS_CLIENT_WRITE_SINGLE_REG})
add_library(emodbus-client-write-multi-reg-static STATIC ${EMODBUS_CLIENT_WRITE_MULTI_REGS})
add_library(emodbus-client-read-write-regs-static STATIC ${EMODBUS_CLIENT_READ_WRITE_REGS})
add_library(emodbus-client-read-file-record-static STATIC ${EMODBUS_CLIENT_READ_FILE_RECORD})
add_library(emodbus-client-write-file-record-static STATIC ${EMODBUS_CLIENT_WRITE_FILE_RECORD})
add_library(emodbus-client-read-fifo-static STATIC ${EMODBUS_CLIENT_READ_FIFO})
endif(DEFINED EMB_BUILD_STATIC_LIBS)

endif(DEFINED EMB_BUILD_PARTIAL_LIBRARIES)
