#ifndef GEO_NETWORK_CLIENT_INITIATETOTALBALANCESFROMREMOUTNODETRANSACTION_H
#define GEO_NETWORK_CLIENT_INITIATETOTALBALANCESFROMREMOUTNODETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/total_balances/TotalBalanceRemouteNodeCommand.h"

class InitiateTotalBalancesFromRemoutNodeTransaction : public BaseTransaction {

public:
    typedef shared_ptr<InitiateTotalBalancesFromRemoutNodeTransaction> Shared;

public:
    InitiateTotalBalancesFromRemoutNodeTransaction(
        NodeUUID &nodeUUID,
        TotalBalanceRemouteNodeCommand::Shared command,
        Logger *logger);

    TotalBalanceRemouteNodeCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    TransactionResult::SharedConst resultOk(
            TrustLineAmount &totalIncomingTrust,
            TrustLineAmount &totalIncomingTrustUsed,
            TrustLineAmount &totalOutgoingTrust,
            TrustLineAmount &totalOutgoingTrustUsed);

private:

    TotalBalanceRemouteNodeCommand::Shared mCommand;

};


#endif //GEO_NETWORK_CLIENT_INITIATETOTALBALANCESFROMREMOUTNODETRANSACTION_H
