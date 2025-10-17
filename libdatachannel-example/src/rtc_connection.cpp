#include "rtc_connection.h"
#include <libdatachannel/rtcdatachannel.h>
#include <libdatachannel/rtcpeerconnection.h>
#include <iostream>

class RTCConnection {
public:
    RTCConnection() {
        // Initialize the peer connection
        rtc::Configuration config;
        peer_connection = rtc::CreatePeerConnection(config);
        
        // Create a data channel
        rtc::DataChannelInit data_channel_config;
        data_channel = peer_connection->CreateDataChannel("dataChannel", data_channel_config);
        
        // Set up data channel event handlers
        data_channel->OnMessage([this](rtc::DataChannelMessage const& msg) {
            handleMessage(msg);
        });
    }

    void connect() {
        // Logic to establish connection
        std::cout << "Connecting..." << std::endl;
        // Additional connection logic goes here
    }

    void sendMessage(const std::string& message) {
        if (data_channel->Send(rtc::DataChannelMessage(message))) {
            std::cout << "Message sent: " << message << std::endl;
        } else {
            std::cerr << "Failed to send message." << std::endl;
        }
    }

private:
    void handleMessage(const rtc::DataChannelMessage& msg) {
        std::cout << "Message received: " << msg.data() << std::endl;
    }

    std::shared_ptr<rtc::PeerConnection> peer_connection;
    std::shared_ptr<rtc::DataChannel> data_channel;
};