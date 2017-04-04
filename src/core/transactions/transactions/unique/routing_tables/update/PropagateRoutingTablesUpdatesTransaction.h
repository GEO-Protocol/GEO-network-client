#ifndef GEO_NETWORK_CLIENT_PROPAGATEROUTINGTABLESUPDATES_H
#define GEO_NETWORK_CLIENT_PROPAGATEROUTINGTABLESUPDATES_H

#include "../../../base/BaseTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/outgoing/routing_tables/RoutingTableUpdateOutgoingMessage.h"
#include "../../../../../network/messages/response/Response.h"

#include "../../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <utility>

class PropagateRoutingTablesUpdatesTransaction: public BaseTransaction {
public:
    typedef shared_ptr<PropagateRoutingTablesUpdatesTransaction> Shared;

    PropagateRoutingTablesUpdatesTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &initiatorUUID,
        const NodeUUID &contractorUUID,
        const TrustLineDirection direction,
        const NodeUUID &recipientUUID,
        const RoutingTableUpdateOutgoingMessage::UpdatingStep updatingStep);

private:
    TransactionResult::SharedConst run();

    pair<bool, TransactionResult::SharedConst> checkContext();

    TransactionResult::SharedConst propagateRoutingTablesUpdates();

    TransactionResult::SharedConst trySendMessageToRecipient();

    void sendMessageToRecipient();

    TransactionResult::SharedConst waitingForResponseFromRecipient();

    void increaseRequestsCounter();

    void progressConnectionTimeout();

private:
    const uint16_t kResponseCodeSuccess = 200;
    const uint16_t kMaxRequestsCount = 5;
    const uint8_t kConnectionProgression = 2;
    const uint32_t kStandardConnectionTimeout = 20000;

    uint16_t mRequestCounter = 0;
    uint32_t mConnectionTimeout = kStandardConnectionTimeout;

    NodeUUID mInitiatorUUID;
    NodeUUID mContractorUUID;
    TrustLineDirection mDirection;
    NodeUUID mRecipientUUID;
    RoutingTableUpdateOutgoingMessage::UpdatingStep mUpdatingStep;

};


#endif //GEO_NETWORK_CLIENT_PROPAGATEROUTINGTABLESUPDATES_H
