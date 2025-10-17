#include <libdatachannel/rtcdatachannel.h>
#include <libdatachannel/rtcpeerconnection.h>

namespace rtc {

DataChannelMessage::DataChannelMessage(std::string data)
    : data_(std::move(data)) {}

const std::string& DataChannelMessage::data() const {
    return data_;
}

void DataChannel::onOpen(OpenCallback callback) {
    openCallback_ = std::move(callback);
}

void DataChannel::onMessage(MessageCallback callback) {
    messageCallback_ = std::move(callback);
}

bool DataChannel::send(const DataChannelMessage& message) {
    if (messageCallback_) {
        messageCallback_(message);
        return true;
    }
    return false;
}

void DataChannel::simulateOpen() {
    if (openCallback_) {
        openCallback_();
    }
}

void DataChannel::simulateIncomingMessage(const std::string& payload) {
    if (messageCallback_) {
        messageCallback_(DataChannelMessage(payload));
    }
}

PeerConnection::PeerConnection(const Configuration& config)
    : config_(config) {}

std::shared_ptr<DataChannel> PeerConnection::createDataChannel(const std::string&, const DataChannelInit&) {
    return std::make_shared<DataChannel>();
}

std::shared_ptr<PeerConnection> createPeerConnection(const Configuration& config) {
    return std::make_shared<PeerConnection>(config);
}

} // namespace rtc

