#include "libdatachannel_example/rtc_connection.h"

#include <iostream>

using libdatachannel_example::RTCConnection;

extern "C" {

int run_demo() {
    RTCConnection connection;

    if (!connection.initialize()) {
        std::cerr << "Failed to initialize RTC connection." << std::endl;
        return 1;
    }

    connection.onMessage([](const std::string& message) {
        std::cout << "WASM app received: " << message << std::endl;
    });

    connection.start();
    connection.sendMessage("Hello from WASM app!");
    connection.processMessages();
    connection.close();
    return 0;
}

}

