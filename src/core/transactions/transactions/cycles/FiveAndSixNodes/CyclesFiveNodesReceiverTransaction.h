#ifndef GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../contractors/ContractorsManager.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesInBetweenMessage.hpp"
#include "../../../../network/messages/cycles/SixAndFiveNodes/CyclesFiveNodesBoundaryMessage.hpp"

class CyclesFiveNodesReceiverTransaction : public BaseTransaction {
public:
    CyclesFiveNodesReceiverTransaction(
        CyclesFiveNodesInBetweenMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

protected:
    CyclesFiveNodesInBetweenMessage::Shared mInBetweenNodeTopologyMessage;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
};
#endif //GEO_NETWORK_CLIENT_CYCLESFIVESIXNODESRESPOSETRANSACTION_H
