#ifndef GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../contractors/ContractorsManager.h"
#include "../../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesNegativeBalanceRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesPositiveBalanceRequestMessage.h"
#include "../../../../network/messages/cycles/FourNodes/CyclesFourNodesBalancesResponseMessage.h"

#include <set>

class CyclesFourNodesReceiverTransaction:
    public BaseTransaction {

public:
    CyclesFourNodesReceiverTransaction(
        CyclesFourNodesNegativeBalanceRequestMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        Logger &logger);

    CyclesFourNodesReceiverTransaction(
        CyclesFourNodesPositiveBalanceRequestMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    void buildSuitableDebtorsForCycleNegativeBalance();

    void buildSuitableDebtorsForCyclePositiveBalance();

    const string logHeader() const override;

protected:
    CyclesFourNodesNegativeBalanceRequestMessage::Shared mRequestMessage;
    ContractorID mNeighborID;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
    bool mNegativeCycleBalance;
    vector<BaseAddress::Shared> mSuitableNodes;
};
#endif //GEO_NETWORK_CLIENT_CYCLEFOURNODESRESPONSETRANSACTION_H
