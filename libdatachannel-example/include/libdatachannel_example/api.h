#ifndef LIBDATACHANNEL_EXAMPLE_API_H
#define LIBDATACHANNEL_EXAMPLE_API_H

namespace libdatachannel_example {

class RTCConnection {
public:
    RTCConnection();
    ~RTCConnection();

    void connect(const std::string& signalingServer);
    void sendData(const std::string& data);
    std::string receiveData();

private:
    // Internal state and methods for managing the RTC connection
};

} // namespace libdatachannel_example

#endif // LIBDATACHANNEL_EXAMPLE_API_H