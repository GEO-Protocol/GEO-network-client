#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../network/messages/total_balances/InitiateTotalBalancesMessage.h"
#include "../../../network/messages/total_balances/TotalBalancesResultMessage.h"

class TotalBalancesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<TotalBalancesTransaction> Shared;

public:
    TotalBalancesTransaction(
        NodeUUID &nodeUUID,
        TotalBalancesCommand::Shared command,
        TrustLinesManager *manager,
        Logger *logger);

    TotalBalancesTransaction(
        NodeUUID &nodeUUID,
        InitiateTotalBalancesMessage::Shared message,
        TrustLinesManager *manager,
        Logger *logger);

    TotalBalancesCommand::Shared command() const;

    InitiateTotalBalancesMessage::Shared message() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:

    TransactionResult::SharedConst resultOk(
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalIncomingTrustUsed,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalOutgoingTrustUsed);

private:

    TotalBalancesCommand::Shared mCommand;
    InitiateTotalBalancesMessage::Shared mMessage;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESTRANSACTION_H
