#ifndef GEO_NETWORK_CLIENT_CONFIRMATIONNOTSTRONGLYREQUIREDMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_CONFIRMATIONNOTSTRONGLYREQUIREDMESSAGESHANDLER_H

#include "../../internal/common/Types.h"
#include "ConfirmationNotStronglyRequiredMessagesQueue.h"
#include "../../../../common/exceptions/RuntimeError.h"
#include "../../../../logger/LoggerMixin.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>

#include <map>


using namespace std;
namespace as = boost::asio;

class ConfirmationNotStronglyRequiredMessagesHandler :
    protected LoggerMixin {

public:
    /**
     * This signal emits every time, when ends timeout of some queue,
     * and messages of this queue must be sent to the remote node once more time.
     *
     * This signal would be emitted for each message in the queue.
     */
    signals::signal<void(pair<NodeUUID, MaxFlowCalculationConfirmationMessage::Shared>)> signalOutgoingMessageReady;

    signals::signal<void(const SerializedEquivalent, const NodeUUID&)> signalClearTopologyCache;

public:
    ConfirmationNotStronglyRequiredMessagesHandler(
        IOService &ioService,
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
        const MaxFlowCalculationConfirmationMessage::Shared confirmationMessage);

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
    void sendPostponedMessages();

protected:
    /**
     * Map is used because it is expected,
     * that queues count would very rarely grow more than 200-300 objects.
     *
     * Current GCC realisation of the "map" and "unordered_map"
     * makes simple "map" faster up to several thousand of items.
     */
    map<pair<SerializedEquivalent, NodeUUID>, ConfirmationNotStronglyRequiredMessagesQueue::Shared> mQueues;

    IOService &mIOService;

    as::steady_timer mCleaningTimer;

    ConfirmationID mCurrentConfirmationID;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMATIONNOTSTRONGLYREQUIREDMESSAGESHANDLER_H
