#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORTRANSACTION_H_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORTRANSACTION_H_H


#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/GetTrustLineCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class GetFirstLevelContractorBalanceTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GetFirstLevelContractorBalanceTransaction> Shared;

public:
    GetFirstLevelContractorBalanceTransaction(
        NodeUUID &nodeUUID,
        GetTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        Logger &logger)
        noexcept;

    GetTrustLineCommand::Shared command() const;

    TransactionResult::SharedConst run();

    TransactionResult::SharedConst resultTrustLineIsAbsent();

protected:
    const string logHeader() const;

private:
    GetTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORTRANSACTION_H_H
