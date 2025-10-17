cmake_minimum_required(VERSION 3.10)
project(libdatachannel_stub)

add_library(datachannel STATIC
    src/rtc_stub.cpp
)

target_include_directories(datachannel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

