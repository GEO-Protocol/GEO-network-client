/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CyclesThreeNodesReceiverTransaction.h"

CyclesThreeNodesReceiverTransaction::CyclesThreeNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesThreeNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_ThreeNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesThreeNodesReceiverTransaction::run() {
    const auto kNeighbors = mRequestMessage->Neighbors();
    stringstream ss;
    // Create message and reserve memory for neighbors
    const auto kMessage = make_shared<CyclesThreeNodesBalancesResponseMessage>(
        mNodeUUID,
        currentTransactionUUID(),
        kNeighbors.size());
    const auto kContractorBalance = mTrustLinesManager->balance(mRequestMessage->senderUUID);
    const TrustLineBalance kZeroBalance = 0;

    if (kContractorBalance == kZeroBalance) {
        return resultDone();
    }

    bool searchDebtors = true;
    if (kContractorBalance > kZeroBalance)
        searchDebtors = false;

    TrustLineBalance stepNodeBalance;
    for (const auto &kNodeUUID: kNeighbors) {
        stepNodeBalance = mTrustLinesManager->balance(kNodeUUID);
        if ((searchDebtors and stepNodeBalance > kZeroBalance)
            or (not searchDebtors and (stepNodeBalance < kZeroBalance)))
            kMessage->addNeighborUUIDAndBalance(
                kNodeUUID);
    }
    if (kMessage->NeighborsAndBalances().size() > 0)
        sendMessage(mRequestMessage->senderUUID, kMessage);
    return resultDone();
}

const string CyclesThreeNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesThreeNodesReceiverTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}

