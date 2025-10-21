#pragma once

#include <optional>
#include <string>
#include <vector>

namespace libdatachannel_example {

struct CandidateEntry {
    std::string sdpMid;
    std::string candidate;
};

struct SignalData {
    std::string type;
    std::string sdp;
    std::vector<CandidateEntry> candidates;
};

std::optional<SignalData> ParseSignalText(const std::string& text);
std::string SerializeSignal(const SignalData& data);

} // namespace libdatachannel_example

