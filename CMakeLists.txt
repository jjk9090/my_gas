cmake_minimum_required(VERSION 2.8)
project(gas)

# 设置 C++ 标准
set(CMAKE_C_STANDARD 99)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_BUILD_TYPE "Debug")

add_definitions(-D__OPTIMIZE__)
if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-D__NO_INLINE__)
endif()

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.c") 
file(GLOB CONFIG "${CMAKE_SOURCE_DIR}/config/*.c") 
add_executable(as-new
    as.c
    ${SOURCES} 
    ${CONFIG} 
)

target_include_directories(as-new PRIVATE
    ${CMAKE_SOURCE_DIR}/../binutils-gdb/include
    ${CMAKE_SOURCE_DIR}/../binutils-gdb/bfd
    ${CMAKE_SOURCE_DIR}/../binutils-gdb/libiberty
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/config
)

target_link_libraries(as-new ${CMAKE_SOURCE_DIR}/../binutils-gdb/bfd/.libs/libbfd.a ${CMAKE_SOURCE_DIR}/../binutils-gdb/zlib/libz.a ${CMAKE_SOURCE_DIR}/../binutils-gdb/libiberty/libiberty.a dl -no-pie)
# 更改可执行文件的输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
target_compile_options(as-new PRIVATE "-std=c99" "-fPIE")