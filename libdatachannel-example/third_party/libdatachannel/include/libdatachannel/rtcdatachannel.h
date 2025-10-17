#pragma once

#include <functional>
#include <queue>
#include <string>

namespace rtc {

class DataChannelMessage {
public:
    explicit DataChannelMessage(std::string data);

    const std::string& data() const;

private:
    std::string data_;
};

class DataChannel {
public:
    using OpenCallback = std::function<void()>;
    using MessageCallback = std::function<void(const DataChannelMessage&)>;

    void onOpen(OpenCallback callback);
    void onMessage(MessageCallback callback);

    bool send(const DataChannelMessage& message);

    void simulateOpen();
    void simulateIncomingMessage(const std::string& payload);

private:
    OpenCallback openCallback_{};
    MessageCallback messageCallback_{};
};

} // namespace rtc

