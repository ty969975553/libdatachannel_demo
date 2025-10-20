#include "libdatachannel_example/rtc_connection.h"

#include <rtc/rtc.hpp>
#include <rtc/peerconnection.hpp>
#include <rtc/datachannel.hpp>
#include <iostream>

namespace libdatachannel_example {

RTCConnection::RTCConnection() = default;

RTCConnection::~RTCConnection() {
    close();
}

bool RTCConnection::initialize() {
    rtc::InitLogger(rtc::LogLevel::Info);
    rtc::Configuration config;
    rtc::DataChannelInit dataChannelConfig;

    peerConnection_ = std::make_shared<rtc::PeerConnection>(config);
    if (!peerConnection_) {
        std::cerr << "Failed to create peer connection." << std::endl;
        return false;
    }

    dataChannel_ = peerConnection_->createDataChannel("demo", dataChannelConfig);
    if (!dataChannel_) {
        std::cerr << "Failed to create data channel." << std::endl;
        return false;
    }

    dataChannel_->onOpen([this]() {
        std::cout << "[INFO] channel open\n";
        running_ = true;
    });

    dataChannel_->onMessage(nullptr,[this](rtc::string message) {
        handleIncomingMessage(message);
    });

    return true;
}

void RTCConnection::start() {
    running_ = true;
}

void RTCConnection::processMessages() {
    while (!messageQueue_.empty()) {
        auto message = messageQueue_.front();
        messageQueue_.pop();
        if (messageCallback_) {
            messageCallback_(message);
        } else {
            std::cout << "Received message: " << message << std::endl;
        }
    }
}

bool RTCConnection::shouldExit() const {
    return !running_;
}

void RTCConnection::close() {
    running_ = false;
    dataChannel_.reset();
    peerConnection_.reset();
}

void RTCConnection::sendMessage(const std::string& message) {
    if (dataChannel_) {
        dataChannel_->send(message);
    }
}

void RTCConnection::onMessage(std::function<void(const std::string&)> callback) {
    messageCallback_ = std::move(callback);
}

void RTCConnection::handleIncomingMessage(const rtc::string& message) {
    messageQueue_.push(message.data());
}

} // namespace libdatachannel_example

