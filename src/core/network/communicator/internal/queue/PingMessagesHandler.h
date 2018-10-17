#ifndef GEO_NETWORK_CLIENT_PINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_PINGMESSAGESHANDLER_H

#include "../../internal/common/Types.h"
#include "../../../../common/exceptions/RuntimeError.h"
#include "../../../../logger/LoggerMixin.hpp"
#include "../../../../io/storage/CommunicatorStorageHandler.h"
#include "../../../../io/storage/CommunicatorIOTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../messages/general/PingMessage.h"
#include "../../../messages/general/PongMessage.h"

#include <boost/asio/steady_timer.hpp>
#include <boost/signals2.hpp>

#include <vector>


using namespace std;
namespace as = boost::asio;

class PingMessagesHandler : protected LoggerMixin {

public:
    /**
     * This signal emits every time, when ends timeout of some queue,
     * and messages of this queue must be sent to the remote node once more time.
     */
    signals::signal<void(pair<NodeUUID, PingMessage::Shared>)> signalOutgoingMessageReady;

public:
    PingMessagesHandler(
        const NodeUUID &nodeUUID,
        IOService &ioService,
        CommunicatorStorageHandler *communicatorStorageHandler,
        Logger &logger);

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
    void tryEnqueueContractor(
        const NodeUUID &contractorUUID);

    /**
     * Tries to find corresponding postponed message to the received confirmation message.
     * In case of success - postponed message would be removed from the queue, as confirmed.
     *
     * @param contractorUUID - UUID of the remote node, that sent confirmation.
     */
    void tryProcessPongMessage(
        const NodeUUID& contractorUUID);

protected:
    const string logHeader() const
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

    void deserializeMessages();

    void delayedRescheduleResendingAfterDeserialization();

protected:
    static const uint16_t kMessagesDeserializationDelayedSecondsTime = 150;
    static const uint16_t kPingMessagesSecondsTimeOut = 60;

private:
    NodeUUID mNodeUUID;
    vector<NodeUUID> mContractors;
    CommunicatorStorageHandler *mCommunicatorStorageHandler;

    IOService &mIOService;

    as::steady_timer mCleaningTimer;

    unique_ptr<as::steady_timer> mDeserializationMessagesTimer;
};


#endif //GEO_NETWORK_CLIENT_PINGMESSAGESHANDLER_H
