#pragma once

#include "libdatachannel_example/rtc_connection.h"

namespace libdatachannel_example {

inline RTCConnection createConnection() {
    RTCConnection connection;
    connection.initialize();
    connection.start();
    return connection;
}

} // namespace libdatachannel_example

