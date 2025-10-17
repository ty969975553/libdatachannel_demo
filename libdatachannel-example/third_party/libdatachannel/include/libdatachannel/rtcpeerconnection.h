#pragma once

#include <functional>
#include <memory>
#include <string>

namespace rtc {

struct Configuration {
};

struct DataChannelInit {
};

class DataChannel;

class PeerConnection {
public:
    explicit PeerConnection(const Configuration& config);

    std::shared_ptr<DataChannel> createDataChannel(const std::string& label,
                                                   const DataChannelInit& init);

private:
    Configuration config_;
};

std::shared_ptr<PeerConnection> createPeerConnection(const Configuration& config);

} // namespace rtc

