#pragma once

#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <rtc/common.hpp>

namespace rtc {
class DataChannel;
class DataChannelMessage;
class PeerConnection;
struct Configuration;
struct DataChannelInit;
} // namespace rtc

namespace libdatachannel_example {

class RTCConnection {
public:
    RTCConnection();
    ~RTCConnection();

    bool initialize();
    void start();
    void processMessages();
    bool shouldExit() const;
    void close();

    void sendMessage(const std::string& message);
    void onMessage(std::function<void(const std::string&)> callback);

private:
    void handleIncomingMessage(const rtc::string& message);

    bool running_{false};
    std::function<void(const std::string&)> messageCallback_{};
    std::shared_ptr<rtc::PeerConnection> peerConnection_;
    std::shared_ptr<rtc::DataChannel> dataChannel_;
    std::queue<std::string> messageQueue_;
};

} // namespace libdatachannel_example

