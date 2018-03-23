#ifndef GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesNegativeBalanceRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesPositiveBalanceRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"

#include <set>

class CyclesFourNodesReceiverTransaction:
    public BaseTransaction {

public:
    CyclesFourNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesFourNodesNegativeBalanceRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    CyclesFourNodesReceiverTransaction(
        const NodeUUID &nodeUUID,
        CyclesFourNodesPositiveBalanceRequestMessage::Shared message,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected:
    void buildSuitableDebtorsForCycleNegativeBalance();

    void buildSuitableDebtorsForCyclePositiveBalance();

    const string logHeader() const;

protected:
    CyclesFourNodesNegativeBalanceRequestMessage::Shared mRequestMessage;
    TrustLinesManager *mTrustLinesManager;
    bool mNegativeCycleBalance;
    vector<NodeUUID> mSuitableNodes;
};
#endif //GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
