/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "ReceiveResultMaxFlowCalculationTransaction.h"

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mSenderIsGateway(false)
{}

ReceiveResultMaxFlowCalculationTransaction::ReceiveResultMaxFlowCalculationTransaction(
    NodeUUID &nodeUUID,
    ResultMaxFlowCalculationGatewayMessage::Shared message,
    TrustLinesManager *manager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::ReceiveResultMaxFlowCalculationTransactionType,
        nodeUUID,
        logger),
    mMessage(message),
    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mSenderIsGateway(true)
{}

ResultMaxFlowCalculationMessage::Shared ReceiveResultMaxFlowCalculationTransaction::message() const
{
    return mMessage;
}

TransactionResult::SharedConst ReceiveResultMaxFlowCalculationTransaction::run()
{
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "initiator: " << mNodeUUID;
    info() << "sender: " << mMessage->senderUUID;
    info() << "sender is gateway: " << mSenderIsGateway;
    info() << "beforeInsert mapTrustLinesCount: " << mMaxFlowCalculationTrustLineManager->trustLinesCounts();
    info() << "receivedTrustLinesOut: " << mMessage->outgoingFlows().size();
#endif

    if (mSenderIsGateway) {
        mMaxFlowCalculationTrustLineManager->addGateway(
            mMessage->senderUUID);
    }

    for (auto const &outgoingFlow : mMessage->outgoingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << outgoingFlow.first << " " << *outgoingFlow.second.get();
#endif
        mMaxFlowCalculationTrustLineManager->addTrustLine(
            make_shared<MaxFlowCalculationTrustLine>(
                mMessage->senderUUID,
                outgoingFlow.first,
                outgoingFlow.second));
    }
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
    info() << "receivedTrustLinesIn: " << mMessage->incomingFlows().size();
#endif
    for (auto const &incomingFlow : mMessage->incomingFlows()) {
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
        info() << "\t" << incomingFlow.first << " " << *incomingFlow.second.get();
#endif
        mMaxFlowCalculationTrustLineManager->addTrustLine(
            make_shared<MaxFlowCalculationTrustLine>(
                incomingFlow.first,
                mMessage->senderUUID,
                incomingFlow.second));
    }
    return resultDone();
}

const string ReceiveResultMaxFlowCalculationTransaction::logHeader() const
{
    stringstream s;
    s << "[ReceiveResultMaxFlowCalculationTA: " << currentTransactionUUID() << "]";
    return s.str();
}

