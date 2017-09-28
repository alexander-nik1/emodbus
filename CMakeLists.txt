
project(emodbus)

cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED PACKAGE_ARCH)
message(WARNING "You are not defined \"PACKAGE_ARCH\" variable.")
endif()

if(NOT DEFINED TARGET_PLATFORM)
message(WARNING "You are not defined \"TARGET_PLATFORM\" variable.")
endif()

if(NOT DEFINED EMB_USE_SYSTEM_BIN_AND_LIB_PATH)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )
endif()

include_directories(emodbus/include)
include_directories(cpp-wrap/include)

if(DEFINED EMB_WITHOUT_DUMPING)
add_definitions(-DEMODBUS_PACKETS_DUMPING=0)
else()
add_definitions(-DEMODBUS_PACKETS_DUMPING=1)
endif()

set(HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
# Disable a warnings around 'offsetof' in C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")

add_subdirectory(emodbus)

add_subdirectory(cpp-wrap)

add_subdirectory(implementations)

if(NOT DEFINED EMB_WITHOUT_TESTS)
add_subdirectory(tests)
endif()

if(NOT DEFINED EMB_WITHOUT_EXAMPLES)
add_subdirectory(examples)
endif()

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

set(CPACK_COMPONENTS_ALL "dev;base;server;client;transport;cpp;impl-posix;full;src")

# component dev
INSTALL(DIRECTORY emodbus/include DESTINATION ${PREFIX}/ COMPONENT dev)
INSTALL(DIRECTORY cpp-wrap/include DESTINATION ${PREFIX}/ COMPONENT dev)
INSTALL(DIRECTORY implementations/include DESTINATION ${PREFIX}/ COMPONENT dev)

# component base
INSTALL(FILES lib/libemodbus-base.so DESTINATION ${PREFIX}/lib/ COMPONENT base)

# component srerver
INSTALL(FILES lib/libemodbus-server.so DESTINATION ${PREFIX}/lib/ COMPONENT server)

# component client
INSTALL(FILES lib/libemodbus-client.so DESTINATION ${PREFIX}/lib/ COMPONENT client)

# component transport
INSTALL(FILES
        lib/libemodbus-transport.so
        DESTINATION ${PREFIX}/lib/ COMPONENT transport)

# component cpp
INSTALL(FILES lib/libemodbus-cpp.so DESTINATION ${PREFIX}/lib/ COMPONENT cpp)

# component impl-posix
INSTALL(FILES
        lib/libemodbus-impl-posix.so
        lib/libemodbus-cpp-impl-posix.so
        DESTINATION ${PREFIX}/lib/ COMPONENT impl-posix)

# component full
INSTALL(FILES
        lib/libemodbus-base.so
        lib/libemodbus-server.so
        lib/libemodbus-client.so
        lib/libemodbus-transport.so
        lib/libemodbus-cpp.so

        lib/libemodbus-impl-posix.so
        lib/libemodbus-cpp-impl-posix.so
        DESTINATION ${PREFIX}/lib/ COMPONENT full)

# component src
INSTALL(DIRECTORY emodbus
        DESTINATION ${PREFIX}/src/
        COMPONENT src)

INSTALL(DIRECTORY cpp-wrap
        DESTINATION ${PREFIX}/src/
        COMPONENT src)

set(MAJOR_VERSION "0")
set(MINOR_VERSION "2")
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

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

include(CPack)
