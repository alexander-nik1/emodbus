
project(emodbus)

cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED EMB_USE_SYSTEM_BIN_AND_LIB_PATH)
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )
endif()

include_directories(
    ./emodbus/include
    ./cpp-wrap/include
)

#set(HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
# Disable a warnings around 'offsetof' in C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")

add_subdirectory(emodbus)

#add_subdirectory(cpp-wrap)

add_subdirectory(implementations)

if(NOT DEFINED EMB_WITHOUT_TESTS)
add_subdirectory(tests)
endif()

if(NOT DEFINED EMB_WITHOUT_EXAMPLES)
#add_subdirectory(examples)
endif()

FILE(GLOB_RECURSE LibFiles "*.hpp" "*.h" README TODO)
add_custom_target(headers SOURCES ${LibFiles})
