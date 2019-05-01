#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../contractors/ContractorsManager.h"

class GetFirstLevelContractorsTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GetFirstLevelContractorsTransaction> Shared;

public:
    GetFirstLevelContractorsTransaction(
        GetFirstLevelContractorsCommand::Shared command,
        TrustLinesManager *trustLinesManager,
        ContractorsManager *contractorsManager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    GetFirstLevelContractorsCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
    ContractorsManager *mContractorsManager;
};

#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
