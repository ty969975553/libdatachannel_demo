cmake_minimum_required(VERSION 3.16)
project(stub_libdatachannel LANGUAGES CXX)

add_library(datachannel STATIC
    src/rtc.cpp
)

target_include_directories(datachannel
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(datachannel PUBLIC cxx_std_17)

