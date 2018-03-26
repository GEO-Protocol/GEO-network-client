/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#include "BaseCollectTopologyTransaction.h"

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    Logger &logger) :

    BaseTransaction(
        type,
        nodeUUID,
        logger),
    mTrustLinesManager(trustLinesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mMaxFlowCalculationNodeCacheManager(maxFlowCalculationNodeCacheManager)
{}

BaseCollectTopologyTransaction::BaseCollectTopologyTransaction(
    const TransactionType type,
    const TransactionUUID &transactionUUID,
    const NodeUUID &nodeUUID,
    TrustLinesManager *trustLinesManager,
    MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    Logger &logger) :

    BaseTransaction(
        type,
        transactionUUID,
        nodeUUID,
        logger),
    mTrustLinesManager(trustLinesManager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mMaxFlowCalculationNodeCacheManager(maxFlowCalculationNodeCacheManager)
{}

TransactionResult::SharedConst BaseCollectTopologyTransaction::run()
{
    switch (mStep) {
        case Stages::SendRequestForCollectingTopology: {
            mStep = Stages::ProcessCollectingTopology;
            return sendRequestForCollectingTopology();
        }
        case Stages::ProcessCollectingTopology: {
            return processCollectingTopology();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                                 "wrong value of mStep");
    }
}

void BaseCollectTopologyTransaction::fillTopology()
{
    while (!mContext.empty()) {
        if (mContext.at(0)->typeID() == Message::MaxFlow_ResultMaxFlowCalculation) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationMessage>();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "Sender " << kMessage->senderUUID << " common";
#endif
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                mMaxFlowCalculationTrustLineManager->addTrustLine(
                    make_shared<MaxFlowCalculationTrustLine>(
                        kMessage->senderUUID,
                        outgoingFlow.first,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                mMaxFlowCalculationTrustLineManager->addTrustLine(
                    make_shared<MaxFlowCalculationTrustLine>(
                        incomingFlow.first,
                        kMessage->senderUUID,
                        incomingFlow.second));
            }
        }
        else if (mContext.at(0)->typeID() == Message::MaxFlow_ResultMaxFlowCalculationFromGateway) {
            const auto kMessage = popNextMessage<ResultMaxFlowCalculationGatewayMessage>();
#ifdef DEBUG_LOG_MAX_FLOW_CALCULATION
            info() << "Sender " << kMessage->senderUUID << " gateway";
#endif
            mMaxFlowCalculationTrustLineManager->addGateway(kMessage->senderUUID);
            for (auto const &outgoingFlow : kMessage->outgoingFlows()) {
                mMaxFlowCalculationTrustLineManager->addTrustLine(
                    make_shared<MaxFlowCalculationTrustLine>(
                        kMessage->senderUUID,
                        outgoingFlow.first,
                        outgoingFlow.second));
            }
            for (auto const &incomingFlow : kMessage->incomingFlows()) {
                mMaxFlowCalculationTrustLineManager->addTrustLine(
                    make_shared<MaxFlowCalculationTrustLine>(
                        incomingFlow.first,
                        kMessage->senderUUID,
                        incomingFlow.second));
            }
        }
        else {
            warning() << "Invalid message type in context during fill topology";
            mContext.pop_front();
        }
    }
}
