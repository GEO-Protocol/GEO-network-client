#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

class TotalBalancesTransaction : public BaseTransaction {

public:
    typedef shared_ptr<TotalBalancesTransaction> Shared;

public:
    TotalBalancesTransaction(
        TotalBalancesCommand::Shared command,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    TransactionResult::SharedConst resultOk(
        const TrustLineAmount &totalIncomingTrust,
        const TrustLineAmount &totalTrustUsedByContractor,
        const TrustLineAmount &totalOutgoingTrust,
        const TrustLineAmount &totalTrustUsedBySelf);

private:
    TotalBalancesCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESTRANSACTION_H
