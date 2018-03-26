/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "CyclesFourNodesReceiverTransaction.h"

CyclesFourNodesReceiverTransaction::CyclesFourNodesReceiverTransaction(
    const NodeUUID &nodeUUID,
    CyclesFourNodesBalancesRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger) :
    BaseTransaction(
        BaseTransaction::TransactionType::Cycles_FourNodesReceiverTransaction,
        message->transactionUUID(),
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mRequestMessage(message)
{}

TransactionResult::SharedConst CyclesFourNodesReceiverTransaction::run()
{
    const auto kDebtorNeighbor = mRequestMessage->debtor();
    const auto kCreditorNeighbor = mRequestMessage->creditor();

    if(mTrustLinesManager->balance(kDebtorNeighbor) <= TrustLine::kZeroBalance())
        return resultDone();

    if(mTrustLinesManager->balance(kCreditorNeighbor) >= TrustLine::kZeroBalance())
        return resultDone();

    sendMessage<CyclesFourNodesBalancesResponseMessage>(
        mRequestMessage->senderUUID,
        mNodeUUID,
        currentTransactionUUID()
    );

    return resultDone();
}

const string CyclesFourNodesReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[CyclesFourNodesReceiverTransactionTA: " << currentTransactionUUID() << "] ";
    return s.str();
}