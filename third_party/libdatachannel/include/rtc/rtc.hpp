#pragma once

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace rtc {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

void InitLogger(LogLevel level) noexcept;

struct string {
    string() = default;
    string(const char* data);
    string(std::string data);

    const char* data() const noexcept;
    std::size_t size() const noexcept;

    operator std::string() const;

    std::string value;
};

std::ostream& operator<<(std::ostream& stream, const string& value);

struct Description {
    enum class Type {
        Offer,
        Answer
    };

    Description() = default;
    Description(std::string sdp, Type type);

    const std::string& sdp() const noexcept;
    const std::string& description() const noexcept;
    std::string generateSdp() const;
    Type type() const noexcept;

private:
    std::string sdp_;
    Type type_{Type::Offer};
};

struct Candidate {
    Candidate() = default;
    Candidate(std::string candidate, std::string mid, int mlineIndex);

    const std::string& candidate() const noexcept;
    std::string mid() const;
    std::string sdpMid() const;
    int mlineIndex() const;
    int sdpMLineIndex() const;

private:
    std::string candidate_;
    std::string mid_;
    int mlineIndex_{0};
};

struct Configuration {
    std::vector<std::string> iceServers;
};

struct DataChannelInit {
};

class DataChannel : public std::enable_shared_from_this<DataChannel> {
public:
    using MessageCallback = std::function<void(rtc::string)>;

    DataChannel();

    void onOpen(std::function<void()> callback);
    void onMessage(MessageCallback callback);
    void onMessage(std::nullptr_t, MessageCallback callback);

    void send(const std::string& message);

private:
    void notifyOpen();

    std::function<void()> onOpenCallback_;
    MessageCallback onMessageCallback_;
    bool opened_{false};

    friend class PeerConnection;
};

class PeerConnection : public std::enable_shared_from_this<PeerConnection> {
public:
    explicit PeerConnection(const Configuration& config);

    std::shared_ptr<DataChannel> createDataChannel(const std::string& label);
    std::shared_ptr<DataChannel> createDataChannel(const std::string& label,
                                                   const DataChannelInit& init);

    void onLocalDescription(std::function<void(Description)> callback);
    void onLocalCandidate(std::function<void(Candidate)> callback);
    void onDataChannel(std::function<void(std::shared_ptr<DataChannel>)> callback);

    void setLocalDescription();
    void setLocalDescription(const Description& description);
    void setRemoteDescription(const Description& description);

    void addRemoteCandidate(const Candidate& candidate);

private:
    void emitLocalSignaling();
    void ensureDataChannel();

    Configuration config_;
    std::function<void(Description)> localDescriptionCallback_;
    std::function<void(Candidate)> localCandidateCallback_;
    std::function<void(std::shared_ptr<DataChannel>)> dataChannelCallback_;
    std::shared_ptr<DataChannel> dataChannel_;
    Description localDescription_;
    bool remoteDescriptionSet_{false};
};

} // namespace rtc

