#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESFROMREMOUTNODETRANSACTION_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESFROMREMOUTNODETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/total_balances/TotalBalancesRemouteNodeCommand.h"
#include "../../../network/messages/total_balances/InitiateTotalBalancesMessage.h"
#include "../../../network/messages/total_balances/TotalBalancesResultMessage.h"

class TotalBalancesFromRemoutNodeTransaction : public BaseTransaction {

public:
    typedef shared_ptr<TotalBalancesFromRemoutNodeTransaction> Shared;

public:
    TotalBalancesFromRemoutNodeTransaction(
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

    TransactionResult::SharedConst resultRemoteNodeIsInaccessible();

    TransactionResult::SharedConst checkTransactionContext();

    TransactionResult::SharedConst resultOk(
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalTrustUsedByContractor,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalTrustUsedBySelf);
    
private:
    const uint16_t kConnectionTimeout = 500;
    const uint16_t kMaxRequestsCount = 5;

private:
    TotalBalancesRemouteNodeCommand::Shared mCommand;
    uint16_t mRequestCounter;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESFROMREMOUTNODETRANSACTION_H
