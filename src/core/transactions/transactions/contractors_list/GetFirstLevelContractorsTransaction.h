#ifndef GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
#define GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
#ifndef GEO_NETWORK_CLIENT_GetFirstLevelContractorsTransaction_H
#define GEO_NETWORK_CLIENT_GetFirstLevelContractorsTransaction_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/contractors_list/GetFirstLevelContractorsCommand.h"
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
            Logger *logger);

    GetFirstLevelContractorsCommand::Shared command() const;

    TransactionResult::SharedConst run();

private:
    GetFirstLevelContractorsCommand::Shared mCommand;
    TrustLinesManager *mTrustLinesManager;

};


#endif //GEO_NETWORK_CLIENT_GetFirstLevelContractorsTransaction_H

#endif //GEO_NETWORK_CLIENT_GETFIRSTLEVELCONTRACTORSTRANSACTION_H
