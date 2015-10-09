
project(emodbus)

cmake_minimum_required(VERSION 2.8)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

add_subdirectory(base)
add_subdirectory(client)
add_subdirectory(protocols)

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
set(CPACK_PROTOCOL_COMPONENTS "protocols;protocols-static")

set(CPACK_COMPONENTS_ALL "${CPACK_BASE_COMPONENTS};${CPACK_SERVER_COMPONENTS};${CPACK_CLIENT_COMPONENTS};${CPACK_PROTOCOL_COMPONENTS}")

# component dev
set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "emodbus-client-full, emodbus-server-full")
INSTALL(FILES base/byte-word.h            DESTINATION /usr/include/emodbus/base      COMPONENT dev)
INSTALL(FILES base/common.h               DESTINATION /usr/include/emodbus/base      COMPONENT dev)
INSTALL(FILES base/modbus_errno.h         DESTINATION /usr/include/emodbus/base      COMPONENT dev)
INSTALL(FILES base/modbus_pdu.h           DESTINATION /usr/include/emodbus/base      COMPONENT dev)
INSTALL(FILES base/modbus_proto.h         DESTINATION /usr/include/emodbus/base      COMPONENT dev)
INSTALL(FILES client/modbus_client.h      DESTINATION /usr/include/emodbus/client    COMPONENT dev)
INSTALL(FILES client/modbus_sched.h       DESTINATION /usr/include/emodbus/client    COMPONENT dev)
INSTALL(FILES client/read_holding_regs.h  DESTINATION /usr/include/emodbus/client    COMPONENT dev)
INSTALL(FILES client/write_mask_reg.h     DESTINATION /usr/include/emodbus/client    COMPONENT dev)
INSTALL(FILES client/write_multi_regs.h   DESTINATION /usr/include/emodbus/client    COMPONENT dev)
INSTALL(FILES client/write_single_reg.h   DESTINATION /usr/include/emodbus/client    COMPONENT dev)
INSTALL(FILES protocols/rtu.h             DESTINATION /usr/include/emodbus/protocols COMPONENT dev)

# component base
set(CPACK_DEBIAN_BASE_PACKAGE_DEPENDS "")
INSTALL(PROGRAMS lib/libemodbus-base.so   DESTINATION /usr/lib/ COMPONENT base)

# component base-static
set(CPACK_DEBIAN_BASE-STATIC_PACKAGE_DEPENDS "")
INSTALL(PROGRAMS lib/libemodbus-base-static.a   DESTINATION /usr/lib/ COMPONENT base-static)

# component srerver-libs
set(CPACK_DEBIAN_SERVER-LIBS_PACKAGE_DEPENDS "emodbus-base")

# component srerver-libs-static
set(CPACK_DEBIAN_SERVER-LIBS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")

# component srerver-full
set(CPACK_DEBIAN_SERVER-FULL_PACKAGE_DEPENDS "emodbus-base")

# component client-libs
set(CPACK_DEBIAN_CLIENT-LIBS_PACKAGE_DEPENDS "emodbus-base")
#INSTALL(PROGRAMS lib/libemodbus-client-base.so DESTINATION /usr/lib/ COMPONENT client-libs)
INSTALL(PROGRAMS lib/libemodbus-client-read-holdings.so DESTINATION /usr/lib/ COMPONENT client-libs)
INSTALL(PROGRAMS lib/libemodbus-client-mask-write-reg.so DESTINATION /usr/lib/ COMPONENT client-libs)

# component client-libs-static
set(CPACK_DEBIAN_CLIENT-LIBS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")
#INSTALL(PROGRAMS lib/libemodbus-client-base-static.a DESTINATION /usr/lib/ COMPONENT client-libs-static)
INSTALL(PROGRAMS lib/libemodbus-client-read-holdings-static.a DESTINATION /usr/lib/ COMPONENT client-libs-static)
INSTALL(PROGRAMS lib/libemodbus-client-mask-write-reg-static.a DESTINATION /usr/lib/ COMPONENT client-libs-static)

# component client-full
set(CPACK_DEBIAN_CLIENT-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(PROGRAMS lib/libemodbus-client-full.so DESTINATION /usr/lib/ COMPONENT client-full)

# component protocols
set(CPACK_DEBIAN_PROTOCOLS_PACKAGE_DEPENDS "emodbus-base")
INSTALL(PROGRAMS lib/libemodbus-rtu.so DESTINATION /usr/lib/ COMPONENT protocols)

# component protocols-static
set(CPACK_DEBIAN_PROTOCOLS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(PROGRAMS lib/libemodbus-rtu-static.a DESTINATION /usr/lib/ COMPONENT protocols-static)


set(MAJOR_VERSION "0")
set(MINOR_VERSION "2")
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

