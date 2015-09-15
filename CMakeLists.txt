
project(modbus-master)

cmake_minimum_required(VERSION 2.8)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

SET(FULL_SRC
	modbus-master.h
	byte-word.h
	common.h
	common.c
	read_holding_regs.h
	read_holding_regs.c
	write_single_reg.h
	write_single_reg.c
	write_multi_regs.h
	write_multi_regs.c
)

add_library(modbus-master-full SHARED ${FULL_SRC})
add_library(modbus-master-full-static STATIC ${FULL_SRC})
