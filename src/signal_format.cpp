#include "libdatachannel_example/signal_format.h"

#include <sstream>

namespace libdatachannel_example {
namespace {

constexpr const char* kTypePrefix = "type:";
constexpr const char* kCandidatePrefix = "candidate:";
constexpr const char* kSdpBegin = "sdp-begin";
constexpr const char* kSdpEnd = "sdp-end";

std::string TrimLine(const std::string& line) {
    auto begin = line.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return std::string{};
    }
    auto end = line.find_last_not_of(" \t\r\n");
    return line.substr(begin, end - begin + 1);
}

} // namespace

std::optional<SignalData> ParseSignalText(const std::string& text) {
    SignalData data;

    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        auto trimmed = TrimLine(line);
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed.rfind(kTypePrefix, 0) == 0) {
            data.type = trimmed.substr(std::char_traits<char>::length(kTypePrefix));
            break;
        }
    }

    if (data.type.empty()) {
        return std::nullopt;
    }

    // Expect sdp-begin on the next significant line.
    bool inSdpSection = false;
    std::ostringstream sdp;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        auto trimmed = TrimLine(line);
        if (trimmed.empty()) {
            continue;
        }

        if (!inSdpSection) {
            if (trimmed == kSdpBegin) {
                inSdpSection = true;
            }
            continue;
        }

        if (trimmed == kSdpEnd) {
            break;
        }

        sdp << trimmed << '\n';
    }

    data.sdp = sdp.str();

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        auto trimmed = TrimLine(line);
        if (trimmed.rfind(kCandidatePrefix, 0) != 0) {
            continue;
        }

        auto payload = trimmed.substr(std::char_traits<char>::length(kCandidatePrefix));
        auto separator = payload.find('|');
        if (separator == std::string::npos) {
            continue;
        }

        CandidateEntry entry;
        entry.sdpMid = payload.substr(0, separator);
        entry.candidate = payload.substr(separator + 1);
        if (!entry.candidate.empty()) {
            data.candidates.emplace_back(std::move(entry));
        }
    }

    return data;
}

std::string SerializeSignal(const SignalData& data) {
    std::ostringstream out;
    out << kTypePrefix << data.type << '\n';
    out << kSdpBegin << '\n';
    if (!data.sdp.empty()) {
        out << data.sdp;
        if (data.sdp.back() != '\n') {
            out << '\n';
        }
    }
    out << kSdpEnd << '\n';

    for (const auto& candidate : data.candidates) {
        out << kCandidatePrefix << candidate.sdpMid << '|' << candidate.candidate << '\n';
    }

    return out.str();
}

} // namespace libdatachannel_example

