#ifndef GEO_NETWORK_CLIENT_INITIATETOTALBALANCESFROMREMOUTNODETRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATETOTALBALANCESFROMREMOUTNODETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/total_balances/TotalBalancesRemouteNodeCommand.h"
#include "../../../network/messages/total_balances/InitiateTotalBalancesMessage.h"
#include "../../../network/messages/total_balances/TotalBalancesResultMessage.h"

class InitiateTotalBalancesFromRemoutNodeTransaction : public BaseTransaction {

public:
    typedef shared_ptr<InitiateTotalBalancesFromRemoutNodeTransaction> Shared;

public:
    InitiateTotalBalancesFromRemoutNodeTransaction(
        NodeUUID &nodeUUID,
        TotalBalancesRemouteNodeCommand::Shared command,
        Logger *logger);

    TotalBalancesRemouteNodeCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    void sendMessageToRemoteNode();

    void increaseRequestsCounter();

    TransactionResult::SharedConst waitingForResponseState();

    TransactionResult::SharedConst noResponseResult();

    TransactionResult::SharedConst checkTransactionContext();

    TransactionResult::SharedConst resultOk(
            const TrustLineAmount &totalIncomingTrust,
            const TrustLineAmount &totalIncomingTrustUsed,
            const TrustLineAmount &totalOutgoingTrust,
            const TrustLineAmount &totalOutgoingTrustUsed);

    TransactionResult::SharedConst unexpectedErrorResult();

private:
    const uint16_t kConnectionTimeout = 500;
    const uint16_t kMaxRequestsCount = 5;

private:
    uint16_t mExpectationResponsesCount = 1;
    TotalBalancesRemouteNodeCommand::Shared mCommand;
    uint16_t mRequestCounter;

};


#endif //GEO_NETWORK_CLIENT_INITIATETOTALBALANCESFROMREMOUTNODETRANSACTION_H
