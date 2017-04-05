#include "CyclesBaseFiveSixNodesInitTransaction.h"

CyclesBaseFiveSixNodesInitTransaction::CyclesBaseFiveSixNodesInitTransaction(const TransactionType type,
                                                                     const NodeUUID &nodeUUID,
                                                                     TransactionsScheduler *scheduler,
                                                                     TrustLinesManager *manager,
                                                                     Logger *logger
)
    : UniqueTransaction(type, nodeUUID, scheduler),
      mTrustLinesManager(manager),
      mlogger(logger)
{
};

CyclesBaseFiveSixNodesInitTransaction::CyclesBaseFiveSixNodesInitTransaction(TransactionsScheduler *scheduler)
    : UniqueTransaction(BaseTransaction::TransactionType::Cycles_BaseFiveSixNodesInitTransaction, scheduler) {
}

TransactionResult::SharedConst CyclesBaseFiveSixNodesInitTransaction::run() {
    switch (mStep){
        case Stages::CollectDataAndSendMessage:
            return runCollectDataAndSendMessagesStage();
        case Stages::ParseMessageAndCreateCycles:
            return runParseMessageAndCreateCyclesStage();
        default:
            throw RuntimeError(
                "CyclesBaseFiveSixNodesInitTransaction::run(): "
                    "invalid transaction step.");

    }
}

