#ifndef RTC_CONNECTION_H
#define RTC_CONNECTION_H

#include <libdatachannel/rtcdatachannel.h>
#include <libdatachannel/rtcpeerconnection.h>
#include <string>
#include <functional>

class RTCConnection {
public:
    RTCConnection();
    ~RTCConnection();

    void initialize(const std::string& signalingServerUrl);
    void createOffer(std::function<void(const std::string&)> onOfferCreated);
    void createAnswer(std::function<void(const std::string&)> onAnswerCreated);
    void setRemoteDescription(const std::string& sdp);
    void sendData(const std::string& data);
    void onDataReceived(std::function<void(const std::string&)> callback);

private:
    rtc::RTCPeerConnection* peerConnection;
    rtc::RTCDataChannel* dataChannel;
};

#endif // RTC_CONNECTION_H