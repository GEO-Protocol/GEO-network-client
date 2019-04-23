#include "OutgoingNodesHandler.h"


OutgoingNodesHandler::OutgoingNodesHandler(
    IOService &ioService,
    UDPSocket &socket,
    ContractorsManager *contractorsManager,
    Logger &logger)
    noexcept:

    mIOService(ioService),
    mSocket(socket),
    mContractorsManager(contractorsManager),
    mCleaningTimer(ioService),
    mLog(logger)
{
    rescheduleCleaning();
}

OutgoingRemoteNode *OutgoingNodesHandler::handler(
    const ContractorID contractorID)
    noexcept
{
    auto contractor = mContractorsManager->contractor(contractorID);
    if (0 == mNodes.count(contractorID)) {
        mNodes[contractorID] = make_unique<OutgoingRemoteNode>(
            contractor,
            mSocket,
            mIOService,
            mContractorsManager,
            mLog);
    }

    mLastAccessDateTimes[contractorID] = utc_now();
    return mNodes[contractorID].get();
}

OutgoingRemoteAddressNode *OutgoingNodesHandler::handler(
    const BaseAddress::Shared address)
    noexcept
{
    // todo : check if there is present OutgoingRemoteAddressNode with requested address
    // and if yes get it contractorID and if no create new one.
    if (0 == mAddressNodes.count(address->fullAddress())) {
        mAddressNodes[address->fullAddress()] = make_unique<OutgoingRemoteAddressNode>(
            address,
            mSocket,
            mIOService,
            mContractorsManager,
            mLog);
    }

    mLastAccessDateTimesAddress[address->fullAddress()] = utc_now();
    return mAddressNodes[address->fullAddress()].get();
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
        this->removeOutdatedAddressHandlers();
        this->rescheduleCleaning();
    });
}

void OutgoingNodesHandler::removeOutdatedHandlers()
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

    forward_list<ContractorID> outdatedHandlersIDs;
    size_t totalOutdatedElements = 0;

    for (const auto &nodeIDAndLastAccess : mLastAccessDateTimes) {
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
        mLastAccessDateTimes.clear();

    } else {
        for (const auto &outdatedHandlerID : outdatedHandlersIDs) {
            mLastAccessDateTimes.erase(outdatedHandlerID);
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

void OutgoingNodesHandler::removeOutdatedAddressHandlers()
    noexcept
{
    if (mAddressNodes.empty()) {
        return;
    }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    debug() << "Outdated address nodes handlers removing started";
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

    forward_list<string> outdatedHandlersAddresses;
    size_t totalOutdatedElements = 0;

    for (const auto &nodeAddressAndLastAccess : mLastAccessDateTimesAddress) {
        if (kNow - nodeAddressAndLastAccess.second < kMaxIdleTimeout) {
            continue;
        }

        // Handler that doesn't sent all it's data must not be removed,
        // even if it is obsolete by the access time.
        if (mAddressNodes[nodeAddressAndLastAccess.first]->containsPacketsInQueue()) {
            continue;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
        debug() << "Remote address node handler for the (" << nodeAddressAndLastAccess.first << ") is outdated. Dropped.";
#endif

        outdatedHandlersAddresses.push_front(nodeAddressAndLastAccess.first);
        ++totalOutdatedElements;
    }


    if (totalOutdatedElements == mAddressNodes.size()) {
        mAddressNodes.clear();
        mLastAccessDateTimesAddress.clear();

    } else {
        for (const auto &outdatedHandlerAddress : outdatedHandlersAddresses) {
            mLastAccessDateTimesAddress.erase(outdatedHandlerAddress);
            mAddressNodes.erase(outdatedHandlerAddress);
        }
    }


#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR_GARBAGE_COLLECTOR
    if (!mAddressNodes.empty()) {
        debug() << mAddressNodes.size() << " address nodes handler(s) are (is) alive";
    }
    debug() << "Outdated address nodes handlers removing finished";
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
