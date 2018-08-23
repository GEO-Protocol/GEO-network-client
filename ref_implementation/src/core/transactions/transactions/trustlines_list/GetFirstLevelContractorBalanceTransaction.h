/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
