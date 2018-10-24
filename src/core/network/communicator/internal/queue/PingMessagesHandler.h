#ifndef GEO_NETWORK_CLIENT_PINGMESSAGESHANDLER_H
#define GEO_NETWORK_CLIENT_PINGMESSAGESHANDLER_H

#include "../../internal/common/Types.h"
#include "../../../../common/exceptions/RuntimeError.h"
#include "../../../../logger/LoggerMixin.hpp"
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
        Logger &logger);

    /**
     * @param contractorUUID - remote node UUID.
     */
    void tryEnqueueContractor(
        const NodeUUID &contractorUUID);

    void enqueueContractorWithPostponedSending(
        const NodeUUID &contractorUUID);

    /**
     * @param contractorUUID - UUID of the remote node, that sent pong message.
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
    void sendPingMessages() const;

    void delayedRescheduleResending();

protected:
    static const uint16_t kMessagesReschedulingSecondsTime = 150;
    static const uint16_t kPingMessagesSecondsTimeOut = 60;

private:
    NodeUUID mNodeUUID;
    vector<NodeUUID> mContractors;

    IOService &mIOService;
    as::steady_timer mResendingTimer;
    unique_ptr<as::steady_timer> mReschedulingTimer;
};


#endif //GEO_NETWORK_CLIENT_PINGMESSAGESHANDLER_H
