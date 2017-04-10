#ifndef GEO_NETWORK_CLIENT_FROMFIRSTLEVELTOSECONDLEVELROUTINGTABLETRANSACTION_H
#define GEO_NETWORK_CLIENT_FROMFIRSTLEVELTOSECONDLEVELROUTINGTABLETRANSACTION_H

#include "../RoutingTablesTransaction.h"

#include "../../../../../common/Types.h"
#include "../../../../../common/NodeUUID.h"

#include "../../../../../network/messages/Message.hpp"
#include "../../../../../network/messages/incoming/routing_tables/FirstLevelRoutingTableIncomingMessage.h"
#include "../../../../../network/messages/response/RoutingTablesResponse.h"

#include "../../../../../io/storage/StorageHandler.h"

#include "../../../../../common/exceptions/ConflictError.h"

#include <stdint.h>
#include <utility>

class FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction : public RoutingTablesTransaction {
public:
    typedef shared_ptr<FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction> Shared;

public:
    FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
        const NodeUUID &nodeUUID,
        FirstLevelRoutingTableIncomingMessage::Shared message,
        StorageHandler *storageHandler,
        Logger *logger);

    FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction(
        BytesShared buffer,
        StorageHandler *storageHandler,
        Logger *logger);

    FirstLevelRoutingTableIncomingMessage::Shared message() const;

    TransactionResult::SharedConst run();

private:
    void saveFirstLevelRoutingTable();

    void sendResponseToContractor(
        const NodeUUID &contractorUUID,
        const uint16_t code);

    const string logHeader() const;

private:
    FirstLevelRoutingTableIncomingMessage::Shared mFirstLevelMessage;

    StorageHandler *mStorageHandler;

};

#endif //GEO_NETWORK_CLIENT_FROMFIRSTLEVELTOSECONDLEVELROUTINGTABLETRANSACTION_H
