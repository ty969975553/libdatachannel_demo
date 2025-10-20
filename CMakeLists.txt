cmake_minimum_required(VERSION 3.16)
project(libdatachannel_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
add_subdirectory(third_party/datachannel-wasm EXCLUDE_FROM_ALL)
else()
add_subdirectory(third_party/libdatachannel)
endif()

add_library(libdatachannel_example STATIC
    src/rtc_connection.cpp
)

target_include_directories(libdatachannel_example
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(libdatachannel_example
    PUBLIC
        datachannel
)

add_executable(native_app
    src/main_native.cpp
)

target_link_libraries(native_app
    PRIVATE
        libdatachannel_example
)

option(ENABLE_WASM "Build the WebAssembly demo" OFF)

if(ENABLE_WASM)
    add_executable(wasm_app
        src/main_wasm.cpp
    )

    target_link_libraries(wasm_app
        PRIVATE
            libdatachannel_example
    )
endif()

enable_testing()

add_executable(connection_tests
    tests/test_connection.cpp
)

target_link_libraries(connection_tests
    PRIVATE
        libdatachannel_example
)

add_test(NAME connection_tests COMMAND connection_tests)

install(TARGETS native_app
    RUNTIME DESTINATION bin
)

if(TARGET wasm_app)
    install(TARGETS wasm_app
        RUNTIME DESTINATION bin
    )
endif()

