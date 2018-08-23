/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "GetFirstLevelContractorsBalancesTransaction.h"

GetFirstLevelContractorsBalancesTransaction::GetFirstLevelContractorsBalancesTransaction(
    NodeUUID &nodeUUID,
    GetTrustLinesCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
    noexcept:
    BaseTransaction(
        BaseTransaction::TrustlinesList,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetTrustLinesCommand::Shared GetFirstLevelContractorsBalancesTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorsBalancesTransaction::run() {
    const auto kNeighborsCount = mTrustLinesManager->trustLines().size();
    stringstream ss;
    ss << to_string(kNeighborsCount);
    for (const auto kNodeUUIDAndTrustline: mTrustLinesManager->trustLines()) {
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.first;
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.second->incomingTrustAmount();
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.second->outgoingTrustAmount();
        ss << kTokensSeparator;
        ss << kNodeUUIDAndTrustline.second->balance();
    }
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

const string GetFirstLevelContractorsBalancesTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorsBalancesTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
