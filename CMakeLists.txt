
project(emodbus)

cmake_minimum_required(VERSION 2.8)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

#aux_source_directory(include HEADERS_SRC)

include_directories(include)

add_definitions(-DEMODBUS_PACKETS_DUMPING=1)

set(HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(base)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(protocols)

add_subdirectory(test)

add_subdirectory(cpp)

# ==========================================================================================
# DEB packages generation

EXECUTE_PROCESS(COMMAND "date" "+%Y.%m.%d.%H.%M" OUTPUT_VARIABLE CURR_DATETIME)
string(REGEX MATCH "(....).(..).(..).(..).(..)" CURR_DATETIME ${CURR_DATETIME}) # remove newline
message(STATUS "Compilation date = ${CURR_DATETIME}")

set(CPACK_COMPONENTS_IGNORE_GROUPS TRUE)
#set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS TRUE)

set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_DEB_PACKAGE_COMPONENT ON)

set(CPACK_COMPONENTS_GROUPING "IGNORE")


set(CPACK_BASE_COMPONENTS "dev;base;base-static")
set(CPACK_SERVER_COMPONENTS "server-libs;server-libs-static;server-full")
set(CPACK_CLIENT_COMPONENTS "client-libs;client-libs-static;client-full")

set(CPACK_PROTO_RTU_COMPONENTS "proto-rtu")

set(CPACK_PROTO_FULL_COMPONENTS "proto-full;proto-static-full")

set(CPACK_ADD_COMPONENTS "full-cpp")

set(CPACK_PROTOCOL_COMPONENTS "${CPACK_PROTO_RTU_COMPONENTS};${CPACK_PROTO_FULL_COMPONENTS}")

set(CPACK_COMPONENTS_ALL "${CPACK_BASE_COMPONENTS};${CPACK_SERVER_COMPONENTS};${CPACK_CLIENT_COMPONENTS};${CPACK_PROTOCOL_COMPONENTS};${CPACK_ADD_COMPONENTS}")

# component dev
# set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "emodbus-client-full, emodbus-server-full")
INSTALL(DIRECTORY include DESTINATION /usr/ COMPONENT dev)

# component base
# set(CPACK_DEBIAN_BASE_PACKAGE_DEPENDS "")
INSTALL(FILES lib/libemodbus-base.so   DESTINATION /usr/lib/ COMPONENT base)

# component base-static
# set(CPACK_DEBIAN_BASE-STATIC_PACKAGE_DEPENDS "")
INSTALL(FILES lib/libemodbus-base-static.a   DESTINATION /usr/lib/ COMPONENT base-static)

# component srerver-libs
# set(CPACK_DEBIAN_SERVER-LIBS_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES 
		lib/libemodbus-server.so
		lib/libemodbus-server-mask-reg.so
		lib/libemodbus-server-read-coils.so
		lib/libemodbus-server-read-file.so
		lib/libemodbus-server-read-holdings.so
		lib/libemodbus-server-write-coil.so
		lib/libemodbus-server-write-coils.so
		lib/libemodbus-server-write-file.so
		lib/libemodbus-server-write-reg.so
		lib/libemodbus-server-write-regs.so
		DESTINATION /usr/lib/ 
		COMPONENT server-libs)

# component srerver-libs-static
# set(CPACK_DEBIAN_SERVER-LIBS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(FILES 
		lib/libemodbus-server-static.a
		lib/libemodbus-server-mask-reg-static.a
		lib/libemodbus-server-read-coils-static.a
		lib/libemodbus-server-read-file-static.a
		lib/libemodbus-server-read-holdings-static.a
		lib/libemodbus-server-write-coil-static.a
		lib/libemodbus-server-write-coils-static.a
		lib/libemodbus-server-write-file-static.a
		lib/libemodbus-server-write-reg-static.a
		lib/libemodbus-server-write-regs-static.a
		DESTINATION /usr/lib/ 
		COMPONENT server-libs-static)

# component srerver-full
# set(CPACK_DEBIAN_SERVER-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-server-full.so   DESTINATION /usr/lib/ COMPONENT server-full)

# component client-libs
# set(CPACK_DEBIAN_CLIENT-LIBS_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES 
		lib/libemodbus-client.so
		lib/libemodbus-client-mask-write-reg.so
		lib/libemodbus-client-read-coils.so
		lib/libemodbus-client-read-fifo.so
		lib/libemodbus-client-read-file-record.so
		lib/libemodbus-client-read-holdings.so
		lib/libemodbus-client-write-coil.so
		lib/libemodbus-client-write-coils.so
		lib/libemodbus-client-write-file-record.so
		lib/libemodbus-client-write-multi-reg.so
		lib/libemodbus-client-write-single-reg.so
		DESTINATION /usr/lib/ 
		COMPONENT client-libs)

# component client-libs-static
# set(CPACK_DEBIAN_CLIENT-LIBS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(FILES 
		lib/libemodbus-client-static.a
		lib/libemodbus-client-mask-write-reg-static.a
		lib/libemodbus-client-read-coils-static.a
		lib/libemodbus-client-read-fifo-static.a
		lib/libemodbus-client-read-file-record-static.a
		lib/libemodbus-client-read-holdings-static.a
		lib/libemodbus-client-write-coil-static.a
		lib/libemodbus-client-write-coils-static.a
		lib/libemodbus-client-write-file-record-static.a
		lib/libemodbus-client-write-multi-reg-static.a
		lib/libemodbus-client-write-single-reg-static.a
		DESTINATION /usr/lib/ 
		COMPONENT client-libs-static)

# component client-full
# set(CPACK_DEBIAN_CLIENT-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-client-full.so DESTINATION /usr/lib/ COMPONENT client-full)

# component proto-rtu
# set(CPACK_DEBIAN_PROTO-RTU_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-rtu.so DESTINATION /usr/lib/ COMPONENT proto-rtu)

# component proto-full
# set(CPACK_DEBIAN_PROTO-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-rtu.so DESTINATION /usr/lib/ COMPONENT proto-full)

# component proto-static-full
# set(CPACK_DEBIAN_PROTO-STATIC-FULL_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(FILES lib/libemodbus-rtu-static.a DESTINATION /usr/lib/ COMPONENT proto-static-full)

# component full-cpp
INSTALL(FILES lib/libemodbus-full-cpp.so DESTINATION /usr/lib/ COMPONENT full-cpp)

set(MAJOR_VERSION "0")
set(MINOR_VERSION "1")
set(PATCH_VERSION "${CURR_DATETIME}")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "qwerty")	# common depends
set(CPACK_PACKAGE_VENDOR "www.eltecom.ru")
set(CPACK_PACKAGE_CONTACT "Alexander Kovylin alexander-nik1@yandex.ru")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY " Extended Modbus library")
set(CPACK_PACKAGE_DESCRIPTION " Library gives full support of Modbus protocol.")

set(PROJECT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

include(CPack)

