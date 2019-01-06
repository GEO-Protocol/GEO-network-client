#include "ObservingCommunicator.h"

ObservingCommunicator::ObservingCommunicator(
    IOService &ioService,
    vector<pair<string, string>> observersAddressesStr) :
    mIOService(ioService)
{
    if (observersAddressesStr.empty()) {
        throw ValueError("ObservingCommunicator: empty observers list");
    }
    for (const auto &addressStr : observersAddressesStr) {
        if (addressStr.first == "ipv4") {
            try {
                mObservers.push_back(
                    make_shared<IPv4WithPortAddress>(
                        addressStr.second));
            } catch (...) {
                throw ValueError("ObservingCommunicator: can't create own address of type " + addressStr.first);
            }

        } else {
            throw ValueError("ObservingCommunicator: can't create own address. "
                                 "Wrong address type " + addressStr.first);
        }
    }
}

void ObservingCommunicator::sendRequestToObservers(
    Message::Shared message)
{
    // todo : implement me
}