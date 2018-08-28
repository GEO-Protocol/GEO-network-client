/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "RoutingTableResponseTransaction.h"

RoutingTableResponseTransaction::RoutingTableResponseTransaction(
    const NodeUUID &nodeUUID,
    RoutingTableRequestMessage::Shared message,
    TrustLinesManager *manager,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::TransactionType::RoutingTableResponceTransactionType,

        nodeUUID,
        logger),
    mRequestMessage(message),
    mTrustLinesManager(manager)
{}

const string RoutingTableResponseTransaction::logHeader() const
{
    stringstream s;
    s << "[RoutingTableResponseTA: " << currentTransactionUUID() << "] ";
    return s.str();
}

TransactionResult::SharedConst RoutingTableResponseTransaction::run()
{
    if(!mTrustLinesManager->isNeighbor(mRequestMessage->senderUUID)){
        warning() << mRequestMessage->senderUUID << " is not a neighbor. Finish transaction;";
    }
    auto neighbors = mTrustLinesManager->rt1();
    set<NodeUUID> result;
    for(auto node:neighbors){
        if(node == mRequestMessage->senderUUID)
            continue;
        result.insert(node);
    }
    sendMessage<RoutingTableResponseMessage>(
        mRequestMessage->senderUUID,
        mNodeUUID,
        result);
    return resultDone();
}
