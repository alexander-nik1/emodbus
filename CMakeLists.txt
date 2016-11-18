
project(emodbus)

cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED PACKAGE_ARCH)
message(FATAL_ERROR "You are not defined \"PACKAGE_ARCH\" variable.")
endif()

if(NOT DEFINED TARGET_PLATFORM)
message(FATAL_ERROR "You are not defined \"TARGET_PLATFORM\" variable.")
endif()

#set( CMAKE_C_COMPILER gcc )
#set( CMAKE_CXX_COMPILER g++ )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

include_directories(emodbus/include)
include_directories(cpp-wrap/include)

add_definitions(-DEMODBUS_PACKETS_DUMPING=1)

set(HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(emodbus)

add_subdirectory(cpp-wrap)

add_subdirectory(implementations)

add_subdirectory(tests)

add_subdirectory(examples)

FILE(GLOB_RECURSE LibFiles "*.hpp" "*.h" README TODO)
add_custom_target(headers SOURCES ${LibFiles})

# ==========================================================================================
# DEB packages generation

if(NOT DEFINED PREFIX)
set( PREFIX "/usr" )
message(WARNING "A 'PREFIX' variable was NOT set, defaulting it to '${PREFIX}'.")
endif()

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

set(CPACK_TRANSPORT_RTU_COMPONENTS "transport-rtu")

set(CPACK_TRANSPORT_FULL_COMPONENTS "transport-full;transport-static-full")

set(CPACK_TRANSPORT_IMPL_COMPONENTS "posix-rtu")

set(CPACK_ADD_COMPONENTS "cpp")

set(CPACK_IMPL_COMPONENTS "impl-posix")

set(CPACK_TRANSPORTCOL_COMPONENTS "${CPACK_TRANSPORT_RTU_COMPONENTS};${CPACK_TRANSPORT_FULL_COMPONENTS};${CPACK_TRANSPORT_IMPL_COMPONENTS}")

set(CPACK_COMPONENTS_ALL "${CPACK_BASE_COMPONENTS};${CPACK_SERVER_COMPONENTS};${CPACK_CLIENT_COMPONENTS};${CPACK_TRANSPORTCOL_COMPONENTS};${CPACK_ADD_COMPONENTS};${CPACK_IMPL_COMPONENTS}")

# component dev
# set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "emodbus-client-full, emodbus-server-full")
INSTALL(DIRECTORY emodbus/include DESTINATION ${PREFIX}/ COMPONENT dev)
INSTALL(DIRECTORY cpp-wrap/include DESTINATION ${PREFIX}/ COMPONENT dev)
INSTALL(DIRECTORY implementations/include DESTINATION ${PREFIX}/ COMPONENT dev)

# component base
# set(CPACK_DEBIAN_BASE_PACKAGE_DEPENDS "")
INSTALL(FILES lib/libemodbus-base.so   DESTINATION ${PREFIX}/lib/ COMPONENT base)

# component base-static
# set(CPACK_DEBIAN_BASE-STATIC_PACKAGE_DEPENDS "")
INSTALL(FILES lib/libemodbus-base-static.a   DESTINATION ${PREFIX}/lib/ COMPONENT base-static)

# component srerver-libs
# set(CPACK_DEBIAN_SERVER-LIBS_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES 
		lib/libemodbus-server.so
		lib/libemodbus-server-mask-reg.so
                lib/libemodbus-server-read-bits.so
		lib/libemodbus-server-read-file.so
                lib/libemodbus-server-read-regs.so
		lib/libemodbus-server-write-coil.so
		lib/libemodbus-server-write-coils.so
		lib/libemodbus-server-write-file.so
		lib/libemodbus-server-write-reg.so
		lib/libemodbus-server-write-regs.so
		DESTINATION ${PREFIX}/lib/
		COMPONENT server-libs)

# component srerver-libs-static
# set(CPACK_DEBIAN_SERVER-LIBS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(FILES 
		lib/libemodbus-server-static.a
		lib/libemodbus-server-mask-reg-static.a
                lib/libemodbus-server-read-bits-static.a
		lib/libemodbus-server-read-file-static.a
                lib/libemodbus-server-read-regs-static.a
		lib/libemodbus-server-write-coil-static.a
		lib/libemodbus-server-write-coils-static.a
		lib/libemodbus-server-write-file-static.a
		lib/libemodbus-server-write-reg-static.a
		lib/libemodbus-server-write-regs-static.a
		DESTINATION ${PREFIX}/lib/
		COMPONENT server-libs-static)

# component srerver-full
# set(CPACK_DEBIAN_SERVER-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-server-full.so   DESTINATION ${PREFIX}/lib/ COMPONENT server-full)

# component client-libs
# set(CPACK_DEBIAN_CLIENT-LIBS_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES 
		lib/libemodbus-client.so
		lib/libemodbus-client-mask-write-reg.so
                lib/libemodbus-client-read-bits.so
		lib/libemodbus-client-read-fifo.so
		lib/libemodbus-client-read-file-record.so
		lib/libemodbus-client-read-regs.so
		lib/libemodbus-client-write-coil.so
		lib/libemodbus-client-write-coils.so
		lib/libemodbus-client-write-file-record.so
		lib/libemodbus-client-write-multi-reg.so
		lib/libemodbus-client-write-single-reg.so
		DESTINATION ${PREFIX}/lib/
                COMPONENT client-libs)

# component client-libs-static
# set(CPACK_DEBIAN_CLIENT-LIBS-STATIC_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(FILES 
		lib/libemodbus-client-static.a
		lib/libemodbus-client-mask-write-reg-static.a
                lib/libemodbus-client-read-bits-static.a
		lib/libemodbus-client-read-fifo-static.a
		lib/libemodbus-client-read-file-record-static.a
		lib/libemodbus-client-read-regs-static.a
		lib/libemodbus-client-write-coil-static.a
		lib/libemodbus-client-write-coils-static.a
		lib/libemodbus-client-write-file-record-static.a
		lib/libemodbus-client-write-multi-reg-static.a
		lib/libemodbus-client-write-single-reg-static.a
		DESTINATION ${PREFIX}/lib/
		COMPONENT client-libs-static)

# component client-full
# set(CPACK_DEBIAN_CLIENT-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-client-full.so DESTINATION ${PREFIX}/lib/ COMPONENT client-full)

# component posix-rtu
INSTALL(FILES
        lib/libemodbus-serial-rtu.so
        lib/libemodbus-tcp-client-rtu.so
        DESTINATION ${PREFIX}/lib/ COMPONENT posix-rtu)

# component transport-rtu
# set(CPACK_DEBIAN_TRANSPORT-RTU_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES lib/libemodbus-rtu.so DESTINATION ${PREFIX}/lib/ COMPONENT transport-rtu)

# component transport-full
# set(CPACK_DEBIAN_TRANSPORT-FULL_PACKAGE_DEPENDS "emodbus-base")
INSTALL(FILES
        lib/libemodbus-rtu.so
        lib/libemodbus-tcp.so
        DESTINATION ${PREFIX}/lib/ COMPONENT transport-full)

# component transport-static-full
# set(CPACK_DEBIAN_TRANSPORT-STATIC-FULL_PACKAGE_DEPENDS "emodbus-base-static")
INSTALL(FILES
        lib/libemodbus-rtu-static.a
        lib/libemodbus-tcp-static.a
        DESTINATION ${PREFIX}/lib/ COMPONENT transport-static-full)

# component cpp
INSTALL(FILES lib/libemodbus-cpp.so DESTINATION ${PREFIX}/lib/ COMPONENT cpp)

# component impl-posix
INSTALL(FILES
        lib/libemodbus-impl-posix.so
        lib/libemodbus-cpp-impl-posix.so
        DESTINATION ${PREFIX}/lib/ COMPONENT impl-posix)

set(MAJOR_VERSION "0")
set(MINOR_VERSION "1")
set(PATCH_VERSION "${CURR_DATETIME}")

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${PACKAGE_ARCH}")
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "qwerty")	# common depends
set(CPACK_PACKAGE_VENDOR "Alexander Kovylin")
set(CPACK_PACKAGE_CONTACT "Alexander Kovylin alex.northfield@gmail.com")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY " Extendable Modbus library")
set(CPACK_PACKAGE_DESCRIPTION " Library gives full support of Modbus protocol.")

set(PROJECT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

include(CPack)
