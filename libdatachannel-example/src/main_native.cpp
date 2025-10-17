#include <iostream>
#include "rtc_connection.h"

int main(int argc, char* argv[]) {
    // Initialize RTC connection
    RTCConnection rtcConnection;

    // Check if the connection is established
    if (!rtcConnection.initialize()) {
        std::cerr << "Failed to initialize RTC connection." << std::endl;
        return -1;
    }

    // Start communication
    rtcConnection.start();

    // Main loop for handling communication
    while (true) {
        // Process incoming messages
        rtcConnection.processMessages();

        // Check for exit condition (for example, user input)
        if (rtcConnection.shouldExit()) {
            break;
        }
    }

    // Clean up and close the connection
    rtcConnection.close();
    return 0;
}