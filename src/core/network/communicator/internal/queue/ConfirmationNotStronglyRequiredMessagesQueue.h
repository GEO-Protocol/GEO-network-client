#ifndef GEO_NETWORK_CLIENT_CONFIRMATIONNOTSTRONGLYREQUIREDMESSAGESQUEUE_H
#define GEO_NETWORK_CLIENT_CONFIRMATIONNOTSTRONGLYREQUIREDMESSAGESQUEUE_H

#include "../../../../common/time/TimeUtils.h"
#include "../../../messages/base/max_flow_calculation/MaxFlowCalculationMessage.h"
#include "../../../messages/base/max_flow_calculation/MaxFlowCalculationConfirmationMessage.h"
#include "../../../messages/max_flow_calculation/ResultMaxFlowCalculationMessage.h"
#include "../../../messages/max_flow_calculation/ResultMaxFlowCalculationGatewayMessage.h"

#include <map>

class ConfirmationNotStronglyRequiredMessagesQueue {

public:
    typedef shared_ptr<ConfirmationNotStronglyRequiredMessagesQueue> Shared;

public:
    ConfirmationNotStronglyRequiredMessagesQueue(
        const SerializedEquivalent equivalent,
        const NodeUUID &contractorUUID)
        noexcept;

    /**
     * Checks message type, and in case if this message requires confirmation -
     * adds it to the internal queue for further re-sending. Otherwise - does nothing.
     */
    bool enqueue(
        MaxFlowCalculationConfirmationMessage::Shared message,
        ConfirmationID confirmationID);

    /**
     * Cancels resending of the message with transaction UUID = "transactionUUID".
     * @returns true in case if queue was containing appropriate message, otherwise - returns false.
     */
    bool tryProcessConfirmation(
        MaxFlowCalculationConfirmationMessage::Shared confirmationMessage);

    /**
     * @returns date time when next sending attempt must be scheduled.
     */
    const DateTime &nextSendingAttemptDateTime()
        noexcept;

    /**
     * @returns messages, that are enqueued by this queue.
     * On each call, internal timer of next re-sending is exponentially increased.
     */
    const map<ConfirmationID, MaxFlowCalculationConfirmationMessage::Shared> &messages()
        noexcept;

    /**
     * @returns messages count in the queue.
     */
    const size_t size() const
        noexcept;

    bool checkIfNeedResendMessages();

protected:
    /**
     * Sets re-sending timeout to the default value.
     */
    void resetInternalTimeout()
        noexcept;

protected:
    const uint8_t kMaxCountResendingAttempts = 3;

protected:
    // Stores messages queue bykMessagesDeserializationDelayedSecondsTime the ConfirmationID.
    // ConfirmationID is used as key to be able to remove messages from the queue,
    // when appropriate confirmation would be received.
    //
    // Map is used because current GCC realisation of map is faster than unordered_map,
    // until several thousands of items in the map.
    map<ConfirmationID, MaxFlowCalculationConfirmationMessage::Shared> mMessages;

    // Stores timeout that must be waited before next sending attempt.
    // This timeout would be exponentially increased on each sending attempt.
    uint16_t mNextTimeoutSeconds;

    // Stores date time, when messages from this queue must be sent to the remote node.
    // On each sending attempt this timeout must be increased by the mNextTimeoutSeconds.
    DateTime mNextSendingAttemptDateTime;

    NodeUUID mContractorUUID;
    SerializedEquivalent mEquivalent;

    uint8_t mCountResendingAttempts;
};


#endif //GEO_NETWORK_CLIENT_CONFIRMATIONNOTSTRONGLYREQUIREDMESSAGESQUEUE_H
