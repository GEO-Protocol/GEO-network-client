#include "CyclesBaseFiveSixNodesInitTransaction.h"

CyclesBaseFiveSixNodesInitTransaction::CyclesBaseFiveSixNodesInitTransaction(
    const TransactionType type,
    const SerializedEquivalent equivalent,
    ContractorsManager *contractorsManager,
    TrustLinesManager *trustLinesManager,
    CyclesManager *cyclesManager,
    TailManager *tailManager,
    Logger &logger) :
    BaseTransaction(
        type,
        equivalent,
        logger),
    mContractorsManager(contractorsManager),
    mTrustLinesManager(trustLinesManager),
    mCyclesManager(cyclesManager),
    mTailManager(tailManager)
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