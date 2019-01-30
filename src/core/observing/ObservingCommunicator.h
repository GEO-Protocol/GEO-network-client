#ifndef GEO_NETWORK_CLIENT_OBSERVINGCOMMUNICATOR_H
#define GEO_NETWORK_CLIENT_OBSERVINGCOMMUNICATOR_H

#include "../contractors/addresses/IPv4WithPortAddress.h"
#include "messages/ObservingMessage.hpp"
#include "../logger/Logger.h"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ObservingCommunicator {

public:
    ObservingCommunicator(
        IOService &ioService,
        Logger &logger);

    BytesShared sendRequestToObserver(
        IPv4WithPortAddress::Shared observerAddress,
        ObservingMessage::Shared request);

protected:
    static string logHeader();

    LoggerStream error() const;

    LoggerStream debug() const;

    LoggerStream info() const;

private:
    IOService &mIOService;
    Logger &mLogger;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGCOMMUNICATOR_H
