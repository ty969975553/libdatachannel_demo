#include "libdatachannel_example/rtc_connection.h"

#include <cassert>
#include <string>

using libdatachannel_example::RTCConnection;

int main() {
    RTCConnection connection;
    bool messageReceived = false;
    std::string payload;

    assert(connection.initialize());

    connection.onMessage([&](const std::string& message) {
        messageReceived = true;
        payload = message;
    });

    connection.start();
    connection.sendMessage("test");
    connection.processMessages();

    assert(messageReceived);
    assert(payload == "test");

    connection.close();
    return 0;
}

