#ifndef GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
#define GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"
#include "../../../../../common/time/TimeUtils.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/outgoing/routing_tables/FirstLevelRoutingTableOutgoingMessage.h"
#include "../../../../../network/messages/outgoing/routing_tables/SecondLevelRoutingTableOutgoingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../../io/storage/StorageHandler.h"

#include "../../../../../common/exceptions/ConflictError.h"

#include <memory>
#include <vector>
#include <stdint.h>
#include <time.h>

using namespace std;

class FromInitiatorToContractorRoutingTablesPropagationTransaction : public RoutingTablesTransaction {
public:
    typedef shared_ptr<FromInitiatorToContractorRoutingTablesPropagationTransaction> Shared;

public:
    FromInitiatorToContractorRoutingTablesPropagationTransaction(
        const NodeUUID &nodeUUID,
        const NodeUUID &contractorUUID,
        TrustLinesManager *trustLinesManager,
        StorageHandler *storageHandler);

    FromInitiatorToContractorRoutingTablesPropagationTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager,
        StorageHandler *storageHandler,
        Logger *logger = nullptr);

    TransactionResult::SharedConst run();

private:
    pair<bool, TransactionResult::SharedConst> checkContext();

    TransactionResult::SharedConst propagateFirstLevelRoutingTable();

    bool isContractorsCountEnoughForRoutingTablesPropagation();

    TransactionResult::SharedConst trySendFirstLevelRoutingTable();

    void sendFirstLevelRoutingTable();

    TransactionResult::SharedConst propagateSecondLevelRoutingTable();

    TransactionResult::SharedConst trySendSecondLevelRoutingTable();

    void sendSecondLevelRoutingTable();

    TransactionResult::SharedConst waitingForRoutingTablePropagationResponse();

    void prepareToNextStep();

private:
    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;
};


#endif //GEO_NETWORK_CLIENT_PROPAGATIONROUTINGTABLESTRANSACTION_H
