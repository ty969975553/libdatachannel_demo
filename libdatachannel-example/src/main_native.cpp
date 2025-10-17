#include "libdatachannel_example/rtc_connection.h"

#include <iostream>

using libdatachannel_example::RTCConnection;

int main() {
    RTCConnection connection;

    if (!connection.initialize()) {
        std::cerr << "Failed to initialize RTC connection." << std::endl;
        return 1;
    }

    connection.onMessage([](const std::string& message) {
        std::cout << "Native app received: " << message << std::endl;
    });

    connection.start();
    connection.sendMessage("Hello from native app!");

    for (int i = 0; i < 5 && !connection.shouldExit(); ++i) {
        connection.processMessages();
    }

    connection.close();
    return 0;
}

