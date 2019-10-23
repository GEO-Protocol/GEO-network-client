#include "OutgoingNodesHandler.h"


OutgoingNodesHandler::OutgoingNodesHandler(
    IOService &ioService,
    UDPSocket &socket,
    Logger &logger)
    noexcept:

    mIOService(ioService),
    mSocket(socket),
    mCleaningTimer(ioService),
    mLog(logger)
{
    rescheduleCleaning();
}

OutgoingRemoteBaseNode *OutgoingNodesHandler::handler(
    const IPv4WithPortAddress::Shared address)
    noexcept
{
    // check if there is present OutgoingRemoteAddressNode with requested address
    // and if yes get it contractorID and if no create new one.
    if (0 == mNodes.count(address->fullAddress())) {
        mNodes[address->fullAddress()] = make_unique<OutgoingRemoteBaseNode>(
            mSocket,
            mIOService,
            address,
            mLog);
    }

    mLastAccessDateTimesNode[address->fullAddress()] = utc_now();
    return mNodes[address->fullAddress()].get();
}

OutgoingRemoteBaseNode *OutgoingNodesHandler::providerHandler(
    const IPv4WithPortAddress::Shared address)
    noexcept
{
    // check if there is present OutgoingRemoteProvider with requested name
    // and if yes get it contractorID and if no create new one.
    if (0 == mProviders.count(address->fullAddress())) {
        mProviders[address->fullAddress()] = make_unique<OutgoingRemoteBaseNode>(
            mSocket,
            mIOService,
            address,
            mLog);
    }

    mLastAccessDateTimesProvider[address->fullAddress()] = utc_now();
    return mProviders[address->fullAddress()].get();
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
        this->removeOutdatedNodeHandlers();
        this->removeOutdatedProviderHandlers();
        this->rescheduleCleaning();
    });
}

void OutgoingNodesHandler::removeOutdatedNodeHandlers()
    noexcept
{
    if (mNodes.empty()) {
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
    // Channels counter is initialised by 0 every time handler is created.
    // In case if handler for the outgoing node would be created and removed too quickly -
    // this counter would be very often reinitialised to 0,
    // and messages would be sent into the channels, that was already used recently.
    //
    // Depending on the network bandwidth, it may, or may not brings messages collisions.
    // It is recommended to set this parameter to 1-2 minutes.
    const auto kMaxIdleTimeout = boost::posix_time::seconds(kHandlersTTL().count());

    forward_list<string> outdatedHandlersIDs;
    size_t totalOutdatedElements = 0;

    for (const auto &nodeIDAndLastAccess : mLastAccessDateTimesNode) {
        if (kNow - nodeIDAndLastAccess.second < kMaxIdleTimeout) {
            continue;
        }

        // Handler that doesn't sent all it's data must not be removed,
        // even if it is obsolete by the access time.
        if (mNodes[nodeIDAndLastAccess.first]->containsPacketsInQueue()) {
            continue;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        debug() << "Remote node handler for the (" << nodeIDAndLastAccess.first << ") is outdated. Dropped.";
#endif

        outdatedHandlersIDs.push_front(nodeIDAndLastAccess.first);
        ++totalOutdatedElements;
    }

    if (totalOutdatedElements == mNodes.size()) {
        mNodes.clear();
        mLastAccessDateTimesNode.clear();

    } else {
        for (const auto &outdatedHandlerID : outdatedHandlersIDs) {
            mLastAccessDateTimesNode.erase(outdatedHandlerID);
            mNodes.erase(outdatedHandlerID);
        }
    }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    if (!mNodes.empty()) {
        debug() << mNodes.size() << " nodes handler(s) are (is) alive";
    }
    debug() << "Outdated nodes handlers removing finished";
#endif
}

void OutgoingNodesHandler::removeOutdatedProviderHandlers()
    noexcept
{
    if (mProviders.empty()) {
        return;
    }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    debug() << "Outdated provider handlers removing started";
#endif

    const auto kNow = utc_now();

    // It is important for this parameter to be relatively big (minutes).
    // Provider handler contains counter of outgoing channels,
    // that increments of every new outgoing message.
    // This prevents sending of several messages into the same channel
    // (and further collision)
    //
    // Channels counter is initialised by 0 every time handler is created.
    // In case if handler for the outgoing node would be created and removed too quickly -
    // this counter would be very often reinitialised to 0,
    // and messages would be sent into the channels, that was already used recently.
    //
    // Depending on the network bandwidth, it may, or may not brings messages collisions.
    // It is recommended to set this parameter to 1-2 minutes.
    const auto kMaxIdleTimeout = boost::posix_time::seconds(kHandlersTTL().count());

    forward_list<string> outdatedHandlersProvider;
    size_t totalOutdatedElements = 0;

    for (const auto &nodeAddressAndLastAccess : mLastAccessDateTimesProvider) {
        if (kNow - nodeAddressAndLastAccess.second < kMaxIdleTimeout) {
            continue;
        }

        // Handler that doesn't sent all it's data must not be removed,
        // even if it is obsolete by the access time.
        if (mProviders[nodeAddressAndLastAccess.first]->containsPacketsInQueue()) {
            continue;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        debug() << "Provider handler for the (" << nodeAddressAndLastAccess.first << ") is outdated. Dropped.";
#endif

        outdatedHandlersProvider.push_front(nodeAddressAndLastAccess.first);
        ++totalOutdatedElements;
    }

    if (totalOutdatedElements == mProviders.size()) {
        mProviders.clear();
        mLastAccessDateTimesProvider.clear();

    } else {
        for (const auto &outdatedHandlerAddress : outdatedHandlersProvider) {
            mLastAccessDateTimesProvider.erase(outdatedHandlerAddress);
            mProviders.erase(outdatedHandlerAddress);
        }
    }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    if (!mProviders.empty()) {
        debug() << mProviders.size() << " address nodes handler(s) are (is) alive";
    }
    debug() << "Outdated provider handlers removing finished";
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
