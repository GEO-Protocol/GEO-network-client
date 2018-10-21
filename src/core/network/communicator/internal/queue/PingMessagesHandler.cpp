#include "PingMessagesHandler.h"

PingMessagesHandler::PingMessagesHandler(
    const NodeUUID &nodeUUID,
    IOService &ioService,
    CommunicatorStorageHandler *communicatorStorageHandler,
    Logger &logger) :

    LoggerMixin(logger),
    mNodeUUID(nodeUUID),
    mCommunicatorStorageHandler(communicatorStorageHandler),
    mIOService(ioService),
    mCleaningTimer(ioService)
{
    mDeserializationMessagesTimer = make_unique<as::steady_timer>(
        mIOService);
    deserializeMessages();
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
    auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
    ioTransaction->communicatorPingMessagesHandler()->saveRecord(contractorUUID);

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
    debug() << "Contractor " << contractorUUID << " enqueued for ping.";
#endif

    if (mContractors.size() == 1) {
        // First message was added for further re-sending.
        rescheduleResending();
    }
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

    auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
    try {
        ioTransaction->communicatorPingMessagesHandler()->deleteRecord(contractorUUID);

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        debug() << "Pong message from contractor " << contractorUUID << " received.";
#endif

    } catch (IOError &e) {
        this->warning() << "Can't remove contractor from storage. Details: " << e.what();
    }
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

    mCleaningTimer.expires_from_now(chrono::seconds(kPingMessagesSecondsTimeOut));
    mCleaningTimer.async_wait([this] (const boost::system::error_code &e) {

        if (e == boost::asio::error::operation_aborted) {
            return;
        }

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages re-sending started.";
#endif

        this->sendPostponedMessages();
        this->rescheduleResending();

#ifdef DEBUG_LOG_NETWORK_COMMUNICATOR
        this->debug() << "Enqueued messages re-sending finished.";
#endif
    });
}

void PingMessagesHandler::sendPostponedMessages() const
{
    for (const auto &contractorUUID : mContractors) {
        signalOutgoingMessageReady(
            make_pair(
                contractorUUID,
                make_shared<PingMessage>(0, mNodeUUID)));
    }
}

void PingMessagesHandler::deserializeMessages()
{
    auto ioTransaction = mCommunicatorStorageHandler->beginTransaction();
    try {
        mContractors = ioTransaction->communicatorPingMessagesHandler()->allContractors();
    } catch (IOError &e) {
        warning() << "Can't read serialized messages from storage. Details: " << e.message();
        return;
    }
    if (mContractors.empty()) {
        return;
    }
    info() << "Serialized messages count: " << mContractors.size();
    mDeserializationMessagesTimer->expires_from_now(
        chrono::seconds(
            kMessagesDeserializationDelayedSecondsTime));
    mDeserializationMessagesTimer->async_wait(
        boost::bind(
            &PingMessagesHandler::delayedRescheduleResendingAfterDeserialization,
            this));
}

void PingMessagesHandler::delayedRescheduleResendingAfterDeserialization()
{
    mDeserializationMessagesTimer->cancel();
    mDeserializationMessagesTimer = nullptr;
    rescheduleResending();
}

const string PingMessagesHandler::logHeader() const
noexcept
{
    return "[PingMessagesHandler]";
}