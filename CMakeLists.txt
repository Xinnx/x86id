cmake_minimum_required(VERSION 2.8)
project(x86id)

set(SRC_FILES src/cpuid.c)
set(HEADER_FILES include/cpuid.h include/utils.h include/leaf2_descriptors.h)

include_directories(include/)
add_executable(x86id ${SRC_FILES} ${HEADER_FILES})
target_compile_options(x86id PUBLIC -O2 -g -std=gnu11)
install(TARGETS x86id RUNTIME DESTINATION bin)
