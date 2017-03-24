#ifndef GEO_NETWORK_CLIENT_FINDPATHTRANSACTION_H
#define GEO_NETWORK_CLIENT_FINDPATHTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../paths/PathsManager.h"
#include "../../../interface/commands_interface/commands/find_path/FindPathCommand.h"
#include "../../../network/messages/find_path/RequestRoutingTablesMessage.h"
#include "../../../network/messages/find_path/ResultRoutingTablesMessage.h"

#include <vector>

class FindPathTransaction : public BaseTransaction {

public:
    typedef shared_ptr<FindPathTransaction> Shared;

public:
    FindPathTransaction(
        NodeUUID &nodeUUID,
        FindPathCommand::Shared command,
        PathsManager *pathManager,
        Logger *logger);

    FindPathCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    void sendMessageToRemoteNode();

    void increaseRequestsCounter();

    TransactionResult::SharedConst waitingForResponseState();

    TransactionResult::SharedConst noResponseResult();

    TransactionResult::SharedConst checkTransactionContext();

    TransactionResult::SharedConst resultOk(vector<NodeUUID> path);

    TransactionResult::SharedConst unexpectedErrorResult();

private:

    const uint16_t kConnectionTimeout = 500;
    const uint16_t kMaxRequestsCount = 5;

private:

    FindPathCommand::Shared mCommand;
    PathsManager* mPathManager;
    uint16_t mRequestCounter;

};


#endif //GEO_NETWORK_CLIENT_FINDPATHTRANSACTION_H
