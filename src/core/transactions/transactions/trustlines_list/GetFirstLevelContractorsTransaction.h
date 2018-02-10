#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"


class GetFirstLevelContractorsTransaction :
    public BaseTransaction {

public:
    typedef shared_ptr<GetFirstLevelContractorsTransaction> Shared;

public:
    GetFirstLevelContractorsTransaction(
        NodeUUID &nodeUUID,
        GetFirstLevelContractorsCommand::Shared command,
        TrustLinesManager *manager,
        Logger &logger)
    noexcept;

    GetFirstLevelContractorsCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

private:
    GetFirstLevelContractorsCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
