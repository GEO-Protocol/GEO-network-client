#include "CyclesBaseFiveSixNodesInitTransaction.h"

CyclesBaseFiveSixNodesInitTransaction::CyclesBaseFiveSixNodesInitTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    TrustLinesManager *manager,
    CyclesManager *cyclesManager,
    Logger &logger) :
    BaseTransaction(
        type,
        nodeUUID,
        equivalent,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager)
{}

TransactionResult::SharedConst CyclesBaseFiveSixNodesInitTransaction::run() {
    switch (mStep) {
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessagesStage();

        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();

        default:
            throw RuntimeError(
                "CyclesBaseFiveSixNodesInitTransaction::run():"
                    "Invalid transaction step.");
    }
}