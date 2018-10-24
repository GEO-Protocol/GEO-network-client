#include "PingMessagesHandler.h"

PingMessagesHandler::PingMessagesHandler(
    const NodeUUID &nodeUUID,
    IOService &ioService,
    Logger &logger) :

    LoggerMixin(logger),
    mNodeUUID(nodeUUID),
    mIOService(ioService),
    mResendingTimer(ioService)
{
    mReschedulingTimer = make_unique<as::steady_timer>(
        mIOService);
    mReschedulingTimer->expires_from_now(
        chrono::seconds(
            kMessagesReschedulingSecondsTime));
    mReschedulingTimer->async_wait(
        boost::bind(
            &PingMessagesHandler::delayedRescheduleResending,
            this));
}

void PingMessagesHandler::tryEnqueueContractor(
    const NodeUUID &contractorUUID)
{
    if (find(
            mContractors.begin(),
            mContractors.end(),
            contractorUUID) != mContractors.end()) {
        return;
    }

    mContractors.push_back(contractorUUID);

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Contractor " << contractorUUID << " enqueued for ping.";
#endif

    if (mContractors.size() == 1) {
        // First message was added for further re-sending.
        rescheduleResending();
    }
}

void PingMessagesHandler::enqueueContractorWithPostponedSending(
    const NodeUUID &contractorUUID)
{
    if (find(
            mContractors.begin(),
            mContractors.end(),
            contractorUUID) != mContractors.end()) {
        return;
    }

    mContractors.push_back(contractorUUID);

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Contractor " << contractorUUID << " enqueued for postponed ping.";
#endif

}

void PingMessagesHandler::tryProcessPongMessage(
    const NodeUUID& contractorUUID)
{
    auto contractorIt = find(
        mContractors.begin(),
        mContractors.end(),
        contractorUUID);
    if (contractorIt == mContractors.end()) {
#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        warning() << "tryProcessPongMessage: no contractor is present " << contractorUUID;
#endif
        return;
    }

    mContractors.erase(
        contractorIt);

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Pong message from contractor " << contractorUUID << " received.";
#endif
}

void PingMessagesHandler::rescheduleResending()
{
    if (mContractors.empty()) {

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "There are no ping messages present. "
                         "Cleaning would not be scheduled any more.";
#endif

        return;
    }

    mResendingTimer.expires_from_now(chrono::seconds(kPingMessagesSecondsTimeOut));
    mResendingTimer.async_wait([this] (const boost::system::error_code &e) {

        if (e == boost::asio::error::operation_aborted) {
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages re-sending started.";
#endif

        this->sendPingMessages();
        this->rescheduleResending();

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages re-sending finished.";
#endif
    });
}

void PingMessagesHandler::sendPingMessages() const
{
    for (const auto &contractorUUID : mContractors) {
        signalOutgoingMessageReady(
            make_pair(
                contractorUUID,
                make_shared<PingMessage>(0, mNodeUUID)));
    }
}

void PingMessagesHandler::delayedRescheduleResending()
{
    mReschedulingTimer->cancel();
    mReschedulingTimer = nullptr;
    rescheduleResending();
}

const string PingMessagesHandler::logHeader() const
noexcept
{
    return "[PingMessagesHandler]";
}