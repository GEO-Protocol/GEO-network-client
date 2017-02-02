#ifndef GEO_NETWORK_CLIENT_SENDROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_SENDROUTINGTABLESTRANSACTION_H

#include "../UniqueTransaction.h"

#include "../../../../network/messages/Message.h"
#include "../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../network/messages/outgoing/routing_tables/SecondLevelRoutingTableOutgoingMessage.h"

#include <memory>

using namespace std;

class SendRoutingTablesTransaction : public UniqueTransaction {
public:
    typedef shared_ptr<SendRoutingTablesTransaction> Shared;

public:
    SendRoutingTablesTransaction(
        NodeUUID &nodeUUID,
        NodeUUID &contractorUUID,
        TransactionsScheduler *scheduler);

    const NodeUUID &contractorUUID() const;

    TransactionResult::Shared run();

private:
    pair<BytesShared, size_t> serializeToBytes();

    void deserializeFromBytes(
        BytesShared buffer);

    void checkSameTypeTransactions();

    bool checkFirstLevelExchangeContext();

    void sendMessageWithFirstLevelRoutingTable();

    TransactionResult::Shared repeatSecondStep();

    TransactionResult::Shared waitingForFirstLevelRoutingTableAcceptedResponse();

private:
    const uint64_t kConnectionTimeout = 5000;

    const uint16_t kMaxRequestsCount = 5;

    const uint64_t kSecondStepRepeatDelay = 20000;

    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_SENDROUTINGTABLESTRANSACTION_H
