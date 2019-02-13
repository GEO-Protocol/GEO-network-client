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
    BaseAddress::Shared nodeAddress,
    vector<BaseAddress::Shared>& nodeNeighbors)
{
    stringstream ss;
    ss << nodeAddress->fullAddress() << kTokensSeparator << to_string(nodeNeighbors.size());
    for (const auto &neighbor : nodeNeighbors) {
        ss << kTokensSeparator << neighbor->fullAddress();
    }
    return make_shared<Event>(
        EventType::Topology,
        ss.str());
}

Event::Shared Event::initTrustLineEvent(
    BaseAddress::Shared source,
    BaseAddress::Shared destination)
{
    stringstream ss;
    ss << source->fullAddress() << kTokensSeparator << destination->fullAddress();
    return make_shared<Event>(
        EventType::InitTrustLine,
        ss.str());
}

Event::Shared Event::closeTrustLineEvent(
    BaseAddress::Shared source,
    BaseAddress::Shared destination)
{
    stringstream ss;
    ss << source->fullAddress() << kTokensSeparator << destination->fullAddress();
    return make_shared<Event>(
        EventType::CloseTrustLine,
        ss.str());
}

Event::Shared Event::paymentEvent(
    BaseAddress::Shared coordinatorAddress,
    BaseAddress::Shared receiverAddress,
    vector<vector<BaseAddress::Shared>>& paymentPaths)
{
    stringstream ss;
    ss << coordinatorAddress->fullAddress() << kTokensSeparator << receiverAddress->fullAddress();
    for (const auto &path : paymentPaths) {
        for (const auto &nodeAddress : path) {
            ss << kTokensSeparator << nodeAddress->fullAddress();
        }
    }
    return make_shared<Event>(
        EventType::Payment,
        ss.str());
}
