#ifndef GEO_NETWORK_CLIENT_OBSERVINGCOMMUNICATOR_H
#define GEO_NETWORK_CLIENT_OBSERVINGCOMMUNICATOR_H

#include "../communicator/internal/common/Types.h"
#include "../../contractors/addresses/IPv4WithPortAddress.h"
#include "../messages/Message.hpp"

using namespace std;
using namespace boost::asio::ip;

class ObservingCommunicator {

public:
    ObservingCommunicator(
        IOService &ioService,
        vector<pair<string, string>> observersAddressesStr);

    void sendRequestToObservers(
        Message::Shared message);

private:
    IOService &mIOService;
    vector<BaseAddress::Shared> mObservers;
};


#endif //GEO_NETWORK_CLIENT_OBSERVINGCOMMUNICATOR_H
