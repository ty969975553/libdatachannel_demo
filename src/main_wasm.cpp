#include "libdatachannel_example/signal_format.h"

#include <rtc/datachannel.hpp>
#include <rtc/peerconnection.hpp>
#include <rtc/rtc.hpp>

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <optional>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace libdatachannel_example {
namespace {

std::string BuildCandidateKey(const CandidateEntry& entry) {
    return entry.sdpMid + "|" + entry.candidate;
}

} // namespace

class WasmAnswerSession {
public:
    WasmAnswerSession() = default;

    bool initialize() {
        if (initialized_) {
            return true;
        }

        rtc::InitLogger(rtc::LogLevel::Info);

        rtc::Configuration config;
        // config.iceServers.emplace_back("stun:stun.l.google.com:19302");
        peerConnection_ = std::make_shared<rtc::PeerConnection>(config);
        if (!peerConnection_) {
            return false;
        }

        localSignal_.type = "answer";

        peerConnection_->onLocalDescription([this](rtc::Description description) {
            localSignal_.sdp = std::string(description);
            signalDirty_ = true;
        });

        peerConnection_->onLocalCandidate([this](rtc::Candidate candidate) {
            CandidateEntry entry;
            entry.sdpMid = candidate.mid();
            entry.candidate = candidate.candidate();
            localSignal_.candidates.emplace_back(std::move(entry));
            signalDirty_ = true;
        });

        peerConnection_->onDataChannel([this](std::shared_ptr<rtc::DataChannel> channel) {
            dataChannel_ = std::move(channel);
            events_.push("Received data channel from native offerer.");

            dataChannel_->onOpen([this]() {
                channelOpen_ = true;
                events_.push("Data channel open.");
            });

            dataChannel_->onClosed([this]() {
                channelOpen_ = false;
                events_.push("Data channel closed.");
            });

            dataChannel_->onMessage(nullptr, [this](rtc::string message) {
                incomingMessages_.push(message.std());
            });
        });

        peerConnection_->onStateChange([this](rtc::PeerConnection::State state) {
            events_.push("Peer connection state: " + std::to_string(static_cast<int>(state)));
        });

        peerConnection_->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
            events_.push("Gathering state: " + std::to_string(static_cast<int>(state)));
        });

        initialized_ = true;
        return true;
    }

    emscripten::val applyOffer(const std::string& offerText) {
        auto result = emscripten::val::object();

        try {
            if (!initialized_) {
                throw std::runtime_error("Session not initialized");
            }

            auto parsed = ParseSignalText(offerText);
            if (!parsed.has_value()) {
                throw std::runtime_error("Failed to parse offer");
            }

            if (parsed->type != "offer") {
                throw std::runtime_error("Signal is not an offer");
            }

            if (!remoteDescriptionApplied_) {
                if (parsed->sdp.empty()) {
                    throw std::runtime_error("Offer SDP missing");
                }

                peerConnection_->setRemoteDescription(rtc::Description(parsed->sdp, rtc::Description::Type::Offer));
                peerConnection_->setLocalDescription(rtc::Description::Type::Answer);
                remoteDescriptionApplied_ = true;
                events_.push("Applied remote description.");
            }

            for (const auto& entry : parsed->candidates) {
                auto key = BuildCandidateKey(entry);
                if (appliedRemoteCandidates_.insert(key).second) {
                    peerConnection_->addRemoteCandidate(rtc::Candidate(entry.candidate, entry.sdpMid));
                }
            }

            result.set("success", true);
            result.set("appliedRemoteDescription", remoteDescriptionApplied_);
            result.set("appliedRemoteCandidates", static_cast<int>(appliedRemoteCandidates_.size()));
        } catch (const std::exception& error) {
            result.set("success", false);
            result.set("error", error.what());
        }

        return result;
    }

    bool hasSignalUpdate() const {
        return signalDirty_;
    }

    std::string consumeSignal() {
        if (!signalDirty_) {
            return std::string{};
        }
        signalDirty_ = false;
        return SerializeSignal(localSignal_);
    }

    std::vector<std::string> drainEvents() {
        std::vector<std::string> output;
        while (!events_.empty()) {
            output.emplace_back(std::move(events_.front()));
            events_.pop();
        }
        return output;
    }

    std::vector<std::string> drainMessages() {
        std::vector<std::string> output;
        while (!incomingMessages_.empty()) {
            output.emplace_back(std::move(incomingMessages_.front()));
            incomingMessages_.pop();
        }
        return output;
    }

    bool sendMessage(const std::string& message) {
        if (!dataChannel_ || !channelOpen_) {
            return false;
        }
        dataChannel_->send(message);
        return true;
    }

    bool isChannelOpen() const {
        return channelOpen_;
    }

private:
    bool initialized_{false};
    bool remoteDescriptionApplied_{false};
    bool signalDirty_{false};
    bool channelOpen_{false};

    std::shared_ptr<rtc::PeerConnection> peerConnection_;
    std::shared_ptr<rtc::DataChannel> dataChannel_;

    SignalData localSignal_;
    std::set<std::string> appliedRemoteCandidates_;
    std::queue<std::string> incomingMessages_;
    std::queue<std::string> events_;
};

} // namespace libdatachannel_example

EMSCRIPTEN_BINDINGS(libdatachannel_example_module) {
    emscripten::class_<libdatachannel_example::WasmAnswerSession>("WasmAnswerSession")
        .constructor<>()
        .function("initialize", &libdatachannel_example::WasmAnswerSession::initialize)
        .function("applyOffer", &libdatachannel_example::WasmAnswerSession::applyOffer)
        .function("hasSignalUpdate", &libdatachannel_example::WasmAnswerSession::hasSignalUpdate)
        .function("consumeSignal", &libdatachannel_example::WasmAnswerSession::consumeSignal)
        .function("drainEvents", &libdatachannel_example::WasmAnswerSession::drainEvents)
        .function("drainMessages", &libdatachannel_example::WasmAnswerSession::drainMessages)
        .function("sendMessage", &libdatachannel_example::WasmAnswerSession::sendMessage)
        .function("isChannelOpen", &libdatachannel_example::WasmAnswerSession::isChannelOpen);
}

