# libdatachannel-example

This project demonstrates how to use the [libdatachannel](https://github.com/paullouisageneau/libdatachannel) library to establish real-time communication using WebRTC. It includes both a native application and a WebAssembly (WASM) application, showcasing the versatility of the library across different platforms.

## Project Structure

- **CMakeLists.txt**: Configuration file for CMake, defining build settings, target platforms, dependencies, and compilation options.
- **toolchain/emscripten.toolchain.cmake**: Toolchain file for Emscripten, configuring the compilation environment for WebAssembly.
- **src/**: Contains the source code for both the native and WASM applications.
  - **main_native.cpp**: Entry point for the native application, setting up RTC connections and handling local communication.
  - **main_wasm.cpp**: Entry point for the WebAssembly application, setting up RTC connections and handling web communication.
  - **rtc_connection.cpp**: Implements the logic for RTC connections, including establishing connections and sending/receiving data.
  - **rtc_connection.h**: Defines the interface and classes for RTC connections, including methods and properties related to connections.
- **include/libdatachannel_example/api.h**: Public API definitions for the project, intended for use by other modules.
- **examples/**: Contains example clients demonstrating the use of RTC connections.
  - **native_client.cpp**: Example client for local RTC communication.
  - **wasm_client.js**: Example client for RTC communication in a web environment using WebAssembly.
- **tests/**: Contains test code to validate the functionality and stability of RTC connections.
  - **test_connection.cpp**: Tests for the RTC connection functionality.
- **third_party/libdatachannel**: External dependency for libdatachannel, included as a git submodule or source code.
- **tools/**: Contains build scripts for the applications.
  - **build_native.sh**: Script to build the native application executable.
  - **build_wasm.sh**: Script to build the WebAssembly application executable.
- **.gitignore**: Lists files and directories to be ignored by version control.
- **README.md**: Documentation and usage instructions for the project.

## Building the Project

To build the native application, run the following command:

```bash
./tools/build_native.sh
```

To build the WebAssembly application, use:

```bash
./tools/build_wasm.sh
```

## Usage

After building the project, you can run the native application directly on your machine. For the WebAssembly application, you can serve the `wasm_client.js` file using a web server and access it through a web browser.

## Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue for any suggestions or improvements.

## License

This project is licensed under the MIT License. See the LICENSE file for more details.