#include "IncomingNodesHandler.h"


IncomingNodesHandler::IncomingNodesHandler(
    MessagesParser &messagesParser,
    Logger &logger)
    noexcept :

    mMessagesParser(messagesParser),
    mLog(logger)
{}

IncomingRemoteNode* IncomingNodesHandler::handler (
    const UDPEndpoint &endpoint)
    noexcept
{
    const auto k = key(endpoint);

    if (mNodes.count(k) == 0) {
        mNodes.emplace(
            k,
            make_unique<IncomingRemoteNode>(
                endpoint,
                mMessagesParser,
                mLog));
    }

    return mNodes[k].get();
}

void IncomingNodesHandler::removeOutdatedEndpoints()
{
    if (mNodes.size() == 0) {
        return;
    }

    static const auto kMaxTTL = chrono::seconds(10);
    const auto kNow = chrono::steady_clock::now();

#ifdef NETWORK_DEBUG_LOG
    debug() << "Outdated endpoints removing started";
#endif

    forward_list<uint64_t> obsoleteIndexes;
    size_t totalOboleteIndexesCount = 0;

    for (const auto &keyAndNodeHandler : mNodes) {
        if (kNow - keyAndNodeHandler.second->lastUpdated() >= kMaxTTL) {
            obsoleteIndexes.push_front(keyAndNodeHandler.first);
            ++totalOboleteIndexesCount;
        }
    }


    if (totalOboleteIndexesCount == mNodes.size()) {
        mNodes.clear();
        mNodes.shrink_to_fit();

    } else {
        // Prevent map reallocation
        mNodes.reserve(mNodes.size());

        for (const auto &kObsoleteIndex: obsoleteIndexes) {
#ifdef NETWORK_DEBUG_LOG
            const auto kEndpoint = mNodes[kObsoleteIndex]->endpoint();
            debug() << "Remote node handler for the " << kEndpoint << " is outdated. Dropped.";
#endif

            mNodes.erase(kObsoleteIndex);
        }

        mNodes.shrink_to_fit();
    }


#ifdef NETWORK_DEBUG_LOG
    if (mNodes.size() > 0) {
        debug() << mNodes.size() << " enpoint connection(s) are alive.";
    }

    debug() << "Outdated endpoints removing finished";
#endif
}

void IncomingNodesHandler::removeOutdatedChannelsOfPresentEndpoints()
{
#ifdef NETWORK_DEBUG_LOG
    debug() << "Outdated channels removing started";
#endif

    for (const auto &indexAndHandler : mNodes) {
        indexAndHandler.second->dropOutdatedChannels();
    }

#ifdef NETWORK_DEBUG_LOG
    debug() << "Outdated channels removing finished";
#endif
}

/**
 * Returns 8 bytes unsigned interger,
 * where first 4 bytes - are IPv4 address,
 * and the rest 4 bytes (actually only 2) - port.
 */
uint64_t IncomingNodesHandler::key(
    const UDPEndpoint &endpoint)
    noexcept
{
    return
        endpoint.address().to_v4().to_ulong()
        + endpoint.port();
}

LoggerStream IncomingNodesHandler::debug() const
    noexcept
{
#ifdef NETWORK_DEBUG_LOG
    return mLog.debug("IncomingNodesHandler");
#endif

#ifndef NETWORK_DEBUG_LOG
    return LoggerStream();
#endif
}
