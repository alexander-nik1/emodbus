
cmake_minimum_required(VERSION 2.8)

project(emodbus)

if(NOT DEFINED EMB_USE_SYSTEM_BIN_AND_LIB_PATH)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )
endif()

link_directories(${EMB_LINK_DIRECTORIES})

include_directories(
	${EMB_INCLUDE_DIRECTORIES}
    ./emodbus/include
	./implementations/include
	./cpp-wrap/include
)

#set(HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
# Disable a warnings around 'offsetof' in C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")


if(DEFINED EMB_BUILD_STATIC_LIBRARY)
	set(EMB_LIBRARY_TYPE STATIC)
else()
	set(EMB_LIBRARY_TYPE SHARED)
endif()

add_library(emodbus ${EMB_LIBRARY_TYPE}
	emodbus/base/common.c
	emodbus/base/modbus_errno.c
	emodbus/base/bit_array.c

	emodbus/protocols/ascii.c
	emodbus/protocols/rtu.c
	emodbus/protocols/add/crc.c
	emodbus/protocols/tcp.c

	emodbus/client/client.c
	emodbus/client/read_bits.c
	emodbus/client/read_fifo.c
	emodbus/client/read_file_record.c
	emodbus/client/read_regs.c
	emodbus/client/read_write_regs.c
	emodbus/client/write_coil.c
	emodbus/client/write_coils.c
	emodbus/client/write_file_record.c
	emodbus/client/write_mask_reg.c
	emodbus/client/write_multi_regs.c
	emodbus/client/write_single_reg.c

	emodbus/server/server.c
	emodbus/server/exception.c
	emodbus/server/read_bits.c
	emodbus/server/read_fifo.c
	emodbus/server/read_file.c
	emodbus/server/read_regs.c
	emodbus/server/read_write_regs.c
	emodbus/server/write_coil.c
	emodbus/server/write_coils.c
	emodbus/server/write_file.c
	emodbus/server/write_mask_reg.c
	emodbus/server/write_multi_regs.c
	emodbus/server/write_single_reg.c

	implementations/posix/ascii-tty-client.c
#	implementations/posix/client.c
#	implementations/posix/client.cpp
	implementations/posix/dumper.c
#	implementations/posix/mb-rtu-via-serial.c
#	implementations/posix/mb-rtu-via-tcp-client.c
#	implementations/posix/mb-tcp-via-tcp-client.c
#	implementations/posix/mb-tcp-via-tcp-server.c
	implementations/posix/pdu.c
	implementations/posix/serial_port.c
#	implementations/posix/tcp-client-rtu.cpp
	implementations/posix/tcp-client.c
#	implementations/posix/tcp-server-event.c
	implementations/posix/tcp-server.c
)

#add_library(emodbuscpp
#	cpp-wrap/emodbus.cpp
#)

#target_link_libraries(emodbuscpp emodbus)

#add_subdirectory(emodbus)

#add_subdirectory(cpp-wrap)

#add_subdirectory(implementations)

if(NOT DEFINED EMB_WITHOUT_TESTS)
add_subdirectory(tests)
endif()

if(NOT DEFINED EMB_WITHOUT_EXAMPLES)
add_subdirectory(examples)
endif()

FILE(GLOB_RECURSE LibFiles "*.hpp" "*.h" README TODO)
add_custom_target(headers SOURCES ${LibFiles})
