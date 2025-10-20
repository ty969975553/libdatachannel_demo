#include "rtc/rtc.hpp"

#include <utility>

namespace rtc {

void InitLogger(LogLevel) noexcept {}

string::string(const char* data) : value(data ? data : "") {}

string::string(std::string data) : value(std::move(data)) {}

const char* string::data() const noexcept { return value.c_str(); }

std::size_t string::size() const noexcept { return value.size(); }

string::operator std::string() const { return value; }

std::ostream& operator<<(std::ostream& stream, const string& value) {
    stream << value.value;
    return stream;
}

Description::Description(std::string sdp, Type type)
    : sdp_(std::move(sdp)), type_(type) {}

const std::string& Description::sdp() const noexcept { return sdp_; }

const std::string& Description::description() const noexcept { return sdp_; }

std::string Description::generateSdp() const { return sdp_; }

Description::Type Description::type() const noexcept { return type_; }

Candidate::Candidate(std::string candidate, std::string mid, int mlineIndex)
    : candidate_(std::move(candidate)), mid_(std::move(mid)), mlineIndex_(mlineIndex) {}

const std::string& Candidate::candidate() const noexcept { return candidate_; }

std::string Candidate::mid() const { return mid_; }

std::string Candidate::sdpMid() const { return mid_; }

int Candidate::mlineIndex() const { return mlineIndex_; }

int Candidate::sdpMLineIndex() const { return mlineIndex_; }

DataChannel::DataChannel() = default;

void DataChannel::onOpen(std::function<void()> callback) {
    onOpenCallback_ = std::move(callback);
    if (opened_ && onOpenCallback_) {
        onOpenCallback_();
    }
}

void DataChannel::onMessage(MessageCallback callback) {
    onMessageCallback_ = std::move(callback);
}

void DataChannel::onMessage(std::nullptr_t, MessageCallback callback) {
    onMessage(callback);
}

void DataChannel::send(const std::string& message) {
    if (onMessageCallback_) {
        onMessageCallback_(rtc::string(message));
    }
}

void DataChannel::notifyOpen() {
    opened_ = true;
    if (onOpenCallback_) {
        onOpenCallback_();
    }
}

PeerConnection::PeerConnection(const Configuration& config) : config_(config) {}

std::shared_ptr<DataChannel> PeerConnection::createDataChannel(const std::string& label) {
    (void)label;
    ensureDataChannel();
    return dataChannel_;
}

std::shared_ptr<DataChannel> PeerConnection::createDataChannel(const std::string& label,
                                                               const DataChannelInit&) {
    return createDataChannel(label);
}

void PeerConnection::onLocalDescription(std::function<void(Description)> callback) {
    localDescriptionCallback_ = std::move(callback);
}

void PeerConnection::onLocalCandidate(std::function<void(Candidate)> callback) {
    localCandidateCallback_ = std::move(callback);
}

void PeerConnection::onDataChannel(std::function<void(std::shared_ptr<DataChannel>)> callback) {
    dataChannelCallback_ = std::move(callback);
    if (dataChannel_ && remoteDescriptionSet_ && dataChannelCallback_) {
        dataChannelCallback_(dataChannel_);
    }
}

void PeerConnection::setLocalDescription() {
    Description description{"stub-sdp", Description::Type::Offer};
    setLocalDescription(description);
}

void PeerConnection::setLocalDescription(const Description& description) {
    localDescription_ = description;
    emitLocalSignaling();
}

void PeerConnection::setRemoteDescription(const Description&) {
    remoteDescriptionSet_ = true;
    ensureDataChannel();
    if (dataChannelCallback_ && dataChannel_) {
        dataChannelCallback_(dataChannel_);
    }
}

void PeerConnection::addRemoteCandidate(const Candidate&) {}

void PeerConnection::emitLocalSignaling() {
    if (localDescriptionCallback_) {
        localDescriptionCallback_(localDescription_);
    }
    if (localCandidateCallback_) {
        Candidate candidate{"candidate:stub", "0", 0};
        localCandidateCallback_(candidate);
    }
}

void PeerConnection::ensureDataChannel() {
    if (dataChannel_) {
        return;
    }
    dataChannel_ = std::make_shared<DataChannel>();
    dataChannel_->notifyOpen();
    if (remoteDescriptionSet_ && dataChannelCallback_) {
        dataChannelCallback_(dataChannel_);
    }
}

} // namespace rtc

