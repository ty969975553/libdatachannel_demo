project(CMakeLists.txt)

cmake_minimum_required(VERSION 3.10)

# Set the project name
project(libdatachannel_example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add subdirectory for third-party dependencies
add_subdirectory(third_party/libdatachannel)

# Native application
add_executable(native_app src/main_native.cpp src/rtc_connection.cpp)
target_link_libraries(native_app datachannel)

# WebAssembly application
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/toolchain/emscripten.toolchain.cmake)
add_executable(wasm_app src/main_wasm.cpp src/rtc_connection.cpp)
target_link_libraries(wasm_app datachannel)

# Install targets
install(TARGETS native_app wasm_app DESTINATION bin)