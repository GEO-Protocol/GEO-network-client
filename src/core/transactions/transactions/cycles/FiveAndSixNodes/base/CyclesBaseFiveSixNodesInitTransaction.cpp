#include "CyclesBaseFiveSixNodesInitTransaction.h"

CyclesBaseFiveSixNodesInitTransaction::CyclesBaseFiveSixNodesInitTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    Logger *logger) :
    BaseTransaction(
        type,
        nodeUUID,
        logger),
    mTrustLinesManager(manager)
{
};

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