/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

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
        NodeUUID &nodeUUID,
        TotalBalancesCommand::Shared command,
        TrustLinesManager *manager,
        Logger &logger);

    TotalBalancesCommand::Shared command() const;

    TransactionResult::SharedConst run();

protected:
    const string logHeader() const;

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
