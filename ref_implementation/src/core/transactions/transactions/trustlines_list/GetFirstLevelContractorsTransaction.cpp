/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "GetFirstLevelContractorsTransaction.h"

GetFirstLevelContractorsTransaction::GetFirstLevelContractorsTransaction(
    NodeUUID &nodeUUID,
    GetFirstLevelContractorsCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
    noexcept :
    BaseTransaction(
        BaseTransaction::ContractorsList,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetFirstLevelContractorsCommand::Shared GetFirstLevelContractorsTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorsTransaction::run() {
    const auto kNeighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(kNeighborsCount);
    for (const auto kNodeUUIDAndTrustline: mTrustLinesManager->trustLines()) {
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.first;
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(mCommand->resultOk(kResultInfo));
}

const string GetFirstLevelContractorsTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorsTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
