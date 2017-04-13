#ifndef GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H
#define GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/incoming/routing_tables/SecondLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../../io/storage/StorageHandler.h"
#include "../../../../../trust_lines/manager/TrustLinesManager.h"

#include "../propagate/FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction.h"

#include "../../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class FromContractorToFirstLevelRoutingTablesAcceptTransaction : public RoutingTablesTransaction  {
public:
    typedef shared_ptr<FromContractorToFirstLevelRoutingTablesAcceptTransaction> Shared;

public:
    FromContractorToFirstLevelRoutingTablesAcceptTransaction(
        const NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared relationshipsBetweenInitiatorAndContractor,
        TrustLinesManager *trustLinesManager,
        StorageHandler *storageHandler,
        Logger *logger);

    FromContractorToFirstLevelRoutingTablesAcceptTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLinesManager,
        StorageHandler *storageHandler,
        Logger *logger);

    FirstLevelRoutingTableIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void saveLinkBetweenInitiatorAndContractor();

    TransactionResult::SharedConst waitingForSecondLevelRoutingTableState();

    TransactionResult::SharedConst checkIncomingMessageForSecondLevelRoutingTable();

    void saveSecondLevelRoutingTable(
        SecondLevelRoutingTableIncomingMessage::Shared secondLevelMessage);

    void sendResponseToContractor(
        const NodeUUID &contractorUUID,
        const uint16_t code);

    void createFromFirstLevelToSecondLevelRoutingTablesPropagationTransaction();

    const string logHeader() const;

private:
    FirstLevelRoutingTableIncomingMessage::Shared mLinkBetweenInitiatorAndContractor;

    StorageHandler *mStorageHandler;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_FROMCONTRACTORTOFIRSTLEVELROUTINGTABLEACCEPTTRANSACTION_H
