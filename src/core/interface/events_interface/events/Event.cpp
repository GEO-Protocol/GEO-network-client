#include "Event.h"

Event::Event(
    EventType identifier,
    const string &eventInformation):
    mEventIdentifier(identifier),
    mEventInformation(eventInformation)
{}

const BytesShared Event::data() const
{
    string dataStr = to_string(mEventIdentifier) + kTokensSeparator +
                     mEventInformation + kCommandsSeparator;
    BytesShared result = tryMalloc(dataStr.size());
    memcpy(
        result.get(),
        dataStr.data(),
        dataStr.size());
    return result;
}

const size_t Event::dataSize() const
{
    return to_string(mEventIdentifier).size() + mEventInformation.size() + 2;
}

Event::Shared Event::topologyEvent(
    BaseAddress::Shared nodeAddress)
{
    return make_shared<Event>(
        EventType::Topology,
        nodeAddress->fullAddress());
}

Event::Shared Event::paymentEvent(
    BaseAddress::Shared coordinatorAddress,
    BaseAddress::Shared receiverAddress,
    vector<vector<BaseAddress::Shared>>& paymentPaths)
{
    stringstream ss;
    ss << coordinatorAddress->fullAddress() << kTokensSeparator
       << receiverAddress->fullAddress() << kTokensSeparator << to_string(paymentPaths.size());
    for (const auto &path : paymentPaths) {
        for (const auto &nodeAddress : path) {
            ss << kTokensSeparator << nodeAddress->fullAddress();
        }
    }
    return make_shared<Event>(
        EventType::Payment,
        ss.str());
}
