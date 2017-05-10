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
    enum Stages {
        SendRequestForRemouteNode = 1,
        GetResponseFromRemouteNode
    };

protected:
    const string logHeader() const;

private:
    void sendMessageToRemoteNode();

    TransactionResult::SharedConst waitingForResponseState();

    TransactionResult::SharedConst resultRemoteNodeIsInaccessible();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst getRemouteNodeTotalBalances();

    TransactionResult::SharedConst resultOk(
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalTrustUsedByContractor,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalTrustUsedBySelf);
    
private:
    const uint16_t kConnectionTimeout = 500;

private:
    TotalBalancesRemouteNodeCommand::Shared mCommand;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESFROMREMOUTNODETRANSACTION_H
