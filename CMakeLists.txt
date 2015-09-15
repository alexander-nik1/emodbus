
project(modbus-master)

cmake_minimum_required(VERSION 2.8)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

SET(FULL_SRC
	byte-word.h
	common.h
	common.c
	read_holding_regs.h
	read_holding_regs.c
)

add_library(modbus-master-full SHARED ${FULL_SRC})
add_library(modbus-master-full-static STATIC ${FULL_SRC})
