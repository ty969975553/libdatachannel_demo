#include <iostream>
#include <libdatachannel/rtcdatachannel.h>
#include <libdatachannel/rtcpeerconnection.h>

int main() {
    // Create a PeerConnection
    auto config = rtc::Configuration();
    auto peerConnection = rtc::CreatePeerConnection(config);

    // Create a DataChannel
    auto dataChannel = peerConnection->CreateDataChannel("exampleChannel", rtc::DataChannelInit());

    // Set up a callback for when messages are received
    dataChannel->OnMessage([](const rtc::DataChannelMessage& message) {
        std::cout << "Received message: " << message.data() << std::endl;
    });

    // Send a message
    std::string message = "Hello from native client!";
    dataChannel->Send(rtc::DataChannelMessage(message));

    // Keep the application running to listen for messages
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}