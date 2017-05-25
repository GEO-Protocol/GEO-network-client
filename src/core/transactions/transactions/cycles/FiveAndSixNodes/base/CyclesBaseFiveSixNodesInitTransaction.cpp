#include "CyclesBaseFiveSixNodesInitTransaction.h"

CyclesBaseFiveSixNodesInitTransaction::CyclesBaseFiveSixNodesInitTransaction(
    const TransactionType type,
    const NodeUUID &nodeUUID,
    TrustLinesManager *manager,
    CyclesManager *cyclesManager,
    StorageHandler *storageHandler,
    Logger *logger) :
    BaseTransaction(
        type,
        nodeUUID,
        logger),
    mTrustLinesManager(manager),
    mCyclesManager(cyclesManager),
    mStorageHandler(storageHandler)
{};

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