#include <rtc/rtc.hpp>

#include "libdatachannel_example/signal_format.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

namespace {

using libdatachannel_example::CandidateEntry;
using libdatachannel_example::ParseSignalText;
using libdatachannel_example::SerializeSignal;
using libdatachannel_example::SignalData;

constexpr const char* kDefaultSignalDir = "./signals";
constexpr const char* kOfferFileName = "offer.txt";
constexpr const char* kAnswerFileName = "answer.txt";

constexpr std::chrono::milliseconds kPollingInterval{200};

void printUsage(const char* executable) {
    std::cout << "Usage: " << executable
              << " --role <offer|answer> [--signal-dir <path>]" << std::endl;
    std::cout << "The process with role 'offer' writes to offer.txt and reads "
              << "answer.txt inside the signal directory. The 'answer' role "
              << "does the opposite. Use the same directory for both "
              << "processes so they can exchange signaling information." << std::endl;
    std::cout << std::endl;
    std::cout << "Type messages and press Enter to send them over the data "
              << "channel once it is open. Enter '/exit' to terminate." << std::endl;
}

std::string buildSignalPath(const std::filesystem::path& dir,
                            const std::string& fileName) {
    return (dir / fileName).string();
}

bool ensureSignalDirectory(const std::filesystem::path& dir) {
    std::error_code ec;
    if (std::filesystem::exists(dir, ec)) {
        return std::filesystem::is_directory(dir, ec);
    }
    return std::filesystem::create_directories(dir, ec);
}

void writeSignalFile(const std::string& path, const SignalData& data) {
    std::ofstream out(path, std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open signal file for writing: " << path << std::endl;
        return;
    }

    out << SerializeSignal(data);
}

std::optional<SignalData> readSignalFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return std::nullopt;
    }

    std::ostringstream contents;
    contents << in.rdbuf();
    if (!contents) {
        return std::nullopt;
    }

    return ParseSignalText(contents.str());
}

rtc::Description::Type toDescriptionType(const std::string& type) {
    if (type == "offer") {
        return rtc::Description::Type::Offer;
    }
    return rtc::Description::Type::Answer;
}

rtc::Description makeDescription(const SignalData& data) {
    return rtc::Description(data.sdp, toDescriptionType(data.type));
}

rtc::Candidate makeCandidate(const CandidateEntry& entry) {
    return rtc::Candidate(entry.candidate, entry.sdpMid);
}

} // namespace

int main(int argc, char* argv[]) {
    std::string role;
    std::filesystem::path signalDir{kDefaultSignalDir};

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--role" && i + 1 < argc) {
            role = argv[++i];
        } else if (arg == "--signal-dir" && i + 1 < argc) {
            signalDir = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    if (role != "offer" && role != "answer") {
        std::cerr << "You must specify a role using --role <offer|answer>." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    if (!ensureSignalDirectory(signalDir)) {
        std::cerr << "Failed to create or access signal directory: " << signalDir << std::endl;
        return 1;
    }

    const bool isOfferer = role == "offer";
    const std::string localSignalPath = isOfferer ? buildSignalPath(signalDir, kOfferFileName)
                                                  : buildSignalPath(signalDir, kAnswerFileName);
    const std::string remoteSignalPath = isOfferer ? buildSignalPath(signalDir, kAnswerFileName)
                                                   : buildSignalPath(signalDir, kOfferFileName);

    rtc::InitLogger(rtc::LogLevel::Debug);
    rtc::Configuration config;
    //config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    auto peerConnection = std::make_shared<rtc::PeerConnection>(config);
    if (!peerConnection) {
        std::cerr << "Failed to create peer connection." << std::endl;
        return 1;
    }

    SignalData localSignal;
    localSignal.type = isOfferer ? "offer" : "answer";

    std::mutex localSignalMutex;
    std::atomic<bool> remoteDescriptionSet{false};
    std::atomic<bool> shouldExit{false};
    std::atomic<bool> channelOpen{false};
    std::atomic<size_t> appliedRemoteCandidates{0};

    auto updateLocalSignal = [&]() {
        std::lock_guard<std::mutex> lock(localSignalMutex);
        writeSignalFile(localSignalPath, localSignal);
    };

    peerConnection->onLocalDescription([&](rtc::Description description) {
        {
            std::lock_guard<std::mutex> lock(localSignalMutex);
            localSignal.sdp = std::string(description);
        }
        updateLocalSignal();
        std::cout << "Local description written to " << localSignalPath << std::endl;
    });

    peerConnection->onLocalCandidate([&](rtc::Candidate candidate) {
        {
            std::lock_guard<std::mutex> lock(localSignalMutex);
            CandidateEntry entry;
            entry.sdpMid = candidate.mid();
            //entry.sdpMLineIndex = candidate.mlineIndex().value_or(0);
            entry.candidate = candidate.candidate();
            localSignal.candidates.push_back(entry);
        }
        updateLocalSignal();
    });
    peerConnection->onSignalingStateChange([&](rtc::PeerConnection::SignalingState state) {
        std::cout << "[pc] signaling=" << (int)state << std::endl;
        });
    peerConnection->onIceStateChange([&](rtc::PeerConnection::IceState state) {
        std::cout << "[pc] ice=" << (int)state << std::endl;
        });


    std::shared_ptr<rtc::DataChannel> dataChannel;

    auto configureDataChannel = [&](const std::shared_ptr<rtc::DataChannel>& channel) {
        dataChannel = channel;
        dataChannel->onOpen([&]() {
            channelOpen = true;
            std::cout << "[datachannel] open" << std::endl;
            if (isOfferer) {
                dataChannel->send("Hello from the offerer!");
            }
        });

        dataChannel->onMessage(nullptr, [&](rtc::string message) {
            std::cout << "[remote] " << message << std::endl;
        });
    };

    if (isOfferer) {
        auto channel = peerConnection->createDataChannel("demo");
        configureDataChannel(channel);
    } else {
        peerConnection->onDataChannel([&](std::shared_ptr<rtc::DataChannel> channel) {
            std::cout << "Received remote data channel." << std::endl;
            configureDataChannel(channel);
        });
    }

    std::thread signalingWatcher([&]() {
        while (!shouldExit.load()) {
            std::this_thread::sleep_for(kPollingInterval);
            auto signal = readSignalFile(remoteSignalPath);
            if (!signal.has_value()) {
                continue;
            }

            if (!remoteDescriptionSet.load() && !signal->sdp.empty()) {
                try {
                    peerConnection->setRemoteDescription(makeDescription(*signal));
                    remoteDescriptionSet = true;
                    std::cout << "Applied remote description from " << remoteSignalPath << std::endl;
                    if (!isOfferer) {
                        if (peerConnection->signalingState() == rtc::PeerConnection::SignalingState::HaveRemoteOffer) {
                            peerConnection->setLocalDescription(rtc::Description::Type::Answer);
                        }
                    }
                } catch (const std::exception& error) {
                    std::cerr << "Failed to set remote description: " << error.what() << std::endl;
                }
            }

            if (remoteDescriptionSet.load()) {
                auto nextIndex = appliedRemoteCandidates.load();
                while (nextIndex < signal->candidates.size()) {
                    const auto& entry = signal->candidates[nextIndex];
                    try {
                        peerConnection->addRemoteCandidate(makeCandidate(entry));
                        appliedRemoteCandidates.store(++nextIndex);
                    } catch (const std::exception& error) {
                        std::cerr << "Failed to add remote candidate: " << error.what() << std::endl;
                        break;
                    }
                }
            }
        }
    });

    if (isOfferer) {
        if (peerConnection->signalingState() == rtc::PeerConnection::SignalingState::Stable) {
            peerConnection->setLocalDescription(rtc::Description::Type::Offer);
        }
    }

    std::thread inputThread([&]() {
        std::string line;
        while (!shouldExit.load() && std::getline(std::cin, line)) {
            if (line == "/exit") {
                shouldExit = true;
                break;
            }
            if (dataChannel && channelOpen.load()) {
                dataChannel->send(line);
            } else {
                std::cout << "[warn] Data channel not open yet." << std::endl;
            }
        }
        shouldExit = true;
    });

    while (!shouldExit.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (inputThread.joinable()) {
        inputThread.join();
    }
    if (signalingWatcher.joinable()) {
        signalingWatcher.join();
    }

    std::cout << "Shutdown complete." << std::endl;
    return 0;
}

