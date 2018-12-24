#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSBALANCESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/GetTrustLinesCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class GetFirstLevelContractorsBalancesTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GetFirstLevelContractorsBalancesTransaction> Shared;

public:
    GetFirstLevelContractorsBalancesTransaction(
        GetTrustLinesCommand::Shared command,
        TrustLinesManager *manager,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    GetTrustLinesCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSBALANCESTRANSACTION_H
