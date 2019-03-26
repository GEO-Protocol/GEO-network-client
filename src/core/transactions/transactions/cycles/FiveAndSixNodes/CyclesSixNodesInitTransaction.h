#ifndef GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H

#include "base/CyclesBaseFiveSixNodesInitTransaction.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesSixNodesBoundaryMessage.hpp"

class CyclesSixNodesInitTransaction :
        public CyclesBaseFiveSixNodesInitTransaction{

public:
    CyclesSixNodesInitTransaction(
        const SerializedEquivalent equivalent,
        ContractorsManager *contractorsManager,
        TrustLinesManager *trustLinesManager,
        CyclesManager *cyclesManager,
        TailManager &tailManager,
        Logger &logger);

protected:
    const string logHeader() const;

protected:
    TransactionResult::SharedConst runCollectDataAndSendMessagesStage();
    TransactionResult::SharedConst runParseMessageAndCreateCyclesStage();

private:
    TailManager &mTailManager;

};
#endif //GEO_NETWORK_CLIENT_CYCLESSIXNODESINITTRANSACTION_H
