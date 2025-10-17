#include <iostream>
#include <libdatachannel/rtcdatachannel.h>
#include <libdatachannel/rtcpeerconnection.h>

int main() {
    // Create a peer connection
    auto config = rtc::Configuration();
    auto peerConnection = rtc::createPeerConnection(config);

    // Create a data channel
    auto dataChannel = peerConnection->createDataChannel("testChannel", rtc::DataChannelInit());

    // Set up data channel event handlers
    dataChannel->onOpen([]() {
        std::cout << "Data channel opened!" << std::endl;
    });

    dataChannel->onMessage([](const std::string& message) {
        std::cout << "Received message: " << message << std::endl;
    });

    // Example of sending a message
    dataChannel->send("Hello from WebAssembly!");

    // Keep the application running to listen for messages
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}