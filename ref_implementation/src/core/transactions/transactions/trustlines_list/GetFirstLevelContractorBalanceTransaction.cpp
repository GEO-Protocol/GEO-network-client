/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "GetFirstLevelContractorBalanceTransaction.h"

GetFirstLevelContractorBalanceTransaction::GetFirstLevelContractorBalanceTransaction(
    NodeUUID &nodeUUID,
    GetTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    Logger &logger)
noexcept:
    BaseTransaction(
        BaseTransaction::TrustLineOne,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager)
{}

GetTrustLineCommand::Shared GetFirstLevelContractorBalanceTransaction::command() const {
    return mCommand;
}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::run() {
    stringstream ss;
    auto contractorUUID = mCommand->contractorUUID();
    if (!mTrustLinesManager->isNeighbor(contractorUUID)) {
        return resultTrustLineIsAbsent();
    }
    auto kContractorTrustLine = mTrustLinesManager->trustLineReadOnly(contractorUUID);
    ss << contractorUUID << kTokensSeparator;
    ss << kContractorTrustLine->incomingTrustAmount() << kTokensSeparator;
    ss << kContractorTrustLine->outgoingTrustAmount() << kTokensSeparator;
    ss << kContractorTrustLine->balance();
    ss << kCommandsSeparator;
    string kResultInfo = ss.str();
    return transactionResultFromCommand(
        mCommand->resultOk(
            kResultInfo));
}

TransactionResult::SharedConst GetFirstLevelContractorBalanceTransaction::resultTrustLineIsAbsent()
{
    return transactionResultFromCommand(
        mCommand->responseTrustlineIsAbsent());
}

const string GetFirstLevelContractorBalanceTransaction::logHeader() const
{
    stringstream s;
    s << "[GetFirstLevelContractorBalanceTA: " << currentTransactionUUID() << "] ";
    return s.str();
}
