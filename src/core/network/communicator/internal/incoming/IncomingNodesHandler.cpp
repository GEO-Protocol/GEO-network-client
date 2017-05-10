#include "IncomingNodesHandler.h"


IncomingNodesHandler::IncomingNodesHandler(
    MessagesParser &messagesParser,
    Logger &logger)
    noexcept :

    mMessagesParser(messagesParser),
    mLog(logger)
{}

/**
 * @returns handler for processing incoming traffic from the remote node.
 *
 * @param endpoint - UDP endpoint of the remote node.
 */
IncomingRemoteNode* IncomingNodesHandler::handler (
    const UDPEndpoint &endpoint)
    noexcept
{
    const auto kEndpointKey = key(endpoint);

    if (mNodes.count(kEndpointKey) == 0) {
        mNodes.emplace(
            kEndpointKey,
            make_unique<IncomingRemoteNode>(
                endpoint,
                mMessagesParser,
                mLog));
    }

    return mNodes[kEndpointKey].get();
}

/**
 * Drops handlers whose TTL has been expired.
 */
void IncomingNodesHandler::removeOutdatedEndpoints()
{
    if (not mNodes.size()) {
        return;
    }

#ifdef NETWORK_DEBUG_LOG
    debug() << "Outdated endpoints removing started";
#endif

    static const auto kMaxHandlerTTL = chrono::seconds(10);
    const auto kNow = chrono::steady_clock::now();

    forward_list<uint64_t> obsoleteIndexes;
    size_t totalOboleteIndexesCount = 0;

    for (const auto &keyAndNodeHandler : mNodes) {
        if (kNow - keyAndNodeHandler.second->lastUpdated() >= kMaxHandlerTTL) {
            obsoleteIndexes.push_front(keyAndNodeHandler.first);
            ++totalOboleteIndexesCount;
        }
    }


    if (totalOboleteIndexesCount == mNodes.size()) {
        mNodes.clear();
        mNodes.shrink_to_fit();

    } else {
        // Prevent map rehashing on elements dropping
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
