#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSBALANCESTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSBALANCESTRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/contractors_list/GetFirstLevelContractorsBalancesCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

class GetFirstLevelContractorsBalancesTransaction :
        public BaseTransaction {

public:
    typedef shared_ptr<GetFirstLevelContractorsBalancesTransaction> Shared;

public:
    GetFirstLevelContractorsBalancesTransaction(
            NodeUUID &nodeUUID,
            GetFirstLevelContractorsBalancesCommand::Shared command,
            TrustLinesManager *manager,
            Logger *logger);

    GetFirstLevelContractorsBalancesCommand::Shared command() const;

    TransactionResult::SharedConst run();

private:
    GetFirstLevelContractorsBalancesCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;

};

#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSBALANCESTRANSACTION_H
