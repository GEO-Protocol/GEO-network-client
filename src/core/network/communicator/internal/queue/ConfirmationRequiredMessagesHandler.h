#ifndef CONFIRMATIONREQUIREDMESSAGESHANDLER_H
#define CONFIRMATIONREQUIREDMESSAGESHANDLER_H

#include "ConfirmationRequiredMessagesQueue.h"
#include "../../internal/common/Types.h"
#include "../../../../common/exceptions/RuntimeError.h"
#include "../../../../logger/LoggerMixin.hpp"
#include "../../../../io/storage/CommunicatorStorageHandler.h"
#include "../../../../io/storage/CommunicatorIOTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../io/storage/IOTransaction.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>

#include <map>


using namespace std;
namespace as = boost::asio;


/**
 * There are several types of messages, that must be confirmed by the counter part.
 * For example, "SetIncomingTrustLineMessage" must be confirmed as processed by the remote node,
 * otherwise, current node must re-send it until remote node would not process it and send confirmation back.
 *
 * This mechanism allows to force processing of some events by the remote nodes,
 * and is useful for preventing various types of desynchronisations.
 *
 * This handler is used for storing and processing queues of the messages,
 * that must be confirmed by the remote nodes. Messages are sent once in a timeout.
 * This timeout exponentially increases after each one sending attempt,
 * to prevent heavy traffic usage against nodes, that are offline.
 *
 * Each remote node has it's own queue and it's own timeout.
 *
 * ---------------------------------------------------------------------------------------------------------
 * ToDo: "I'm online" message.
 * In case if remote node would be offline long period of time - timeout interval on current node would grow
 * up to 10+ minutes. Theoretically, it is possible, that remote node would come back online right after
 * failed sending attempt, so the next message would be sent to it after 10+ minutes.
 *
 * But, it would be much more efficient, if remote node might send "I'm online message" to it's contractors,
 * and them would know that it's time to try to send postponed messages once more.
 */
class ConfirmationRequiredMessagesHandler:
    protected LoggerMixin {

public:
    /**
     * This signal emits every time, when ends timeout of some queue,
     * and messages of this queue must be sent to the remote node once more time.
     *
     * This signal would be emitted for each message in the queue.
     */
    signals::signal<void(pair<NodeUUID, TransactionMessage::Shared>)> signalOutgoingMessageReady;

public:
    ConfirmationRequiredMessagesHandler(
        IOService &ioService,
        CommunicatorStorageHandler *communicatorStorageHandler,
        Logger &logger)
        noexcept;

    /**
     * Checks "message" type, and in case if this message must be confirmed, -
     * enqueues it for further processing. If there is no queue for the "contractor" -
     * it would be created.
     *
     * (This method might be expended with other messages types).
     *
     * @param contractorUUID - remote node UUID.
     * @param message - message that must be confirmed by the remote node.
     */
    void tryEnqueueMessage(
        const NodeUUID &contractorUUID,
        const Message::Shared message);

    /**
     * Tries to find corresponding postponed message to the received confirmation message.
     * In case of success - postponed message would be removed from the queue, as confirmed.
     *
     * @param contractorUUID - UUID of the remote node, that sent confirmation.
     */
    void tryProcessConfirmation(
        const ConfirmationMessage::Shared confirmationMessage);

protected:
    const string logHeader() const
        noexcept;

    /**
     * @returns timestamp, when next timer awakeness must be performed.
     * This method checks all queues and returns the smallest one time duration,
     * between now and queue timeout.
     */
    const DateTime closestQueueSendingTimestamp() const
        noexcept;

    /**
     * Schedules timer for next awakeness.
     */
    void rescheduleResending();

    /**
     * Sends postponed messages to the remote nodes.
     * This method would be called every time when some queue timeout would fire up.
     */
    void sendPostponedMessages() const;

    void addMessageToStorage(
        const NodeUUID &contractorUUID,
        TransactionMessage::Shared message);

    void removeMessageFromStorage(
        const NodeUUID &contractorUUID,
        const SerializedEquivalent equivalent,
        Message::SerializedType messageType);

    void deserializeMessages();

    void tryEnqueueMessageWithoutConnectingSignalsToSlots(
        const NodeUUID &contractorUUID,
        const TransactionMessage::Shared message);

    void delayedRescheduleResendingAfterDeserialization();

protected:
    static const uint16_t kMessagesDeserializationDelayedSecondsTime = 150;

protected:
    /**
     * Map is used because it is expected,
     * that queues count would very rarely grow more than 200-300 objects.
     *
     * Current GCC realisation of the "map" and "unordered_map"
     * makes simple "map" faster up to several thousand of items.
     */
    map<pair<SerializedEquivalent, NodeUUID>, ConfirmationRequiredMessagesQueue::Shared> mQueues;

    IOService &mIOService;

    CommunicatorStorageHandler *mCommunicatorStorageHandler;

    as::steady_timer mCleaningTimer;

    // this field used for removing and adding messages to storage during enqueue messages
    unique_ptr<CommunicatorIOTransaction> ioTransactionUnique;

    unique_ptr<as::steady_timer> mDeserializationMessagesTimer;
};

#endif // CONFIRMATIONREQUIREDMESSAGESHANDLER_H
