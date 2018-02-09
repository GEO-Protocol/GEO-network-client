#include "OutgoingNodesHandler.h"


OutgoingNodesHandler::OutgoingNodesHandler(
    IOService &ioService,
    UDPSocket &socket,
    UUID2Address &UUID2AddressService,
    Logger &logger)
    noexcept:

    mIOService(ioService),
    mSocket(socket),
    mUUID2AddressService(UUID2AddressService),
    mCleaningTimer(ioService),
    mLog(logger)
{
    rescheduleCleaning();
}

OutgoingRemoteNode *OutgoingNodesHandler::handler(
    const NodeUUID &remoteNodeUUID)
    noexcept
{
    if (0 == mNodes.count(remoteNodeUUID)) {
        mNodes[remoteNodeUUID] = make_unique<OutgoingRemoteNode>(
            remoteNodeUUID,
            mUUID2AddressService,
            mSocket,
            mIOService,
            mLog);
    }

    mLastAccessDateTimes[remoteNodeUUID] = utc_now();
    return mNodes[remoteNodeUUID].get();
}

/**
 * @brief OutgoingNodesHandler::kHandlersTTL
 * @returns timeout that must be wait, before remote node handler would be considered as obsolete.
 */
chrono::seconds OutgoingNodesHandler::kHandlersTTL()
    noexcept
{
    // By default, handler must be dropped, if not new messages has been arrived for 15m.
    static const chrono::seconds kTTL(15 * 60);
    return kTTL;
}

void OutgoingNodesHandler::rescheduleCleaning()
    noexcept
{
    mCleaningTimer.expires_from_now(kHandlersTTL());
    mCleaningTimer.async_wait([this] (const boost::system::error_code&) {
        this->removeOutdatedHandlers();
        this->rescheduleCleaning();
    });
}

void OutgoingNodesHandler::removeOutdatedHandlers()
    noexcept
{
    if (mNodes.size() == 0) {
        return;
    }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    debug() << "Outdated nodes handlers removing started";
#endif


    const auto kNow = utc_now();

    // It is important for this parameter to be relatively big (minutes).
    // Remote node handler contains counter of outgoing channels,
    // that increments of every new outgoing message.
    // This prevents sending of several messages into the same channel
    // (and further collision)
    //
    // Channels counter is initalised by 0 every time handler is created.
    // In case if handler for the outgoing node would be created and removed too quickly -
    // this counter would be very often reinitialised to 0,
    // and messages would be sent into the channels, that was alredy used recently.
    //
    // Depending on the network bandwidth, it may, or may not brings messages collisions.
    // It is recommended to set this paramter to 1-2 minutes.
    const auto kMaxIdleTimeout = boost::posix_time::seconds(kHandlersTTL().count());

    forward_list<NodeUUID> outdatedHandlersUUIDs;
    size_t totalOutdateElements = 0;

    for (const auto &nodeUUIDAndLastAccess : mLastAccessDateTimes) {
        if (kNow - nodeUUIDAndLastAccess.second < kMaxIdleTimeout) {
            continue;
        }

        // Handler that doesn't sent all it's data must not be removed,
        // even if it is obsolete by the access time.
        if (mNodes[nodeUUIDAndLastAccess.first]->containsPacketsInQueue()) {
            continue;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        debug() << "Remote node handler for the (" << nodeUUIDAndLastAccess.first << ") is outdated. Dropped.";
#endif

        outdatedHandlersUUIDs.push_front(nodeUUIDAndLastAccess.first);
        ++totalOutdateElements;
    }


    if (totalOutdateElements == mNodes.size()) {
        mNodes.clear();
        mLastAccessDateTimes.clear();

    } else {
        for (const auto &outdatedHandlerUUID : outdatedHandlersUUIDs) {
            mLastAccessDateTimes.erase(outdatedHandlerUUID);
            mNodes.erase(outdatedHandlerUUID);
        }
    }


#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    if (mNodes.size() > 0) {
        debug() << mNodes.size() << " nodes handler(s) are (is) alive";
    }
    debug() << "Outdated nodes handlers removing finished";
#endif
}

string OutgoingNodesHandler::logHeader()
    noexcept
{
    return "[OutgoingNodesHandler]";
}

LoggerStream OutgoingNodesHandler::debug() const
    noexcept
{
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    return mLog.debug(logHeader());
#endif

#ifndef DEBUG_LOG_NETWORK_COMMUNICATOR
    return LoggerStream::dummy();
#endif
}
