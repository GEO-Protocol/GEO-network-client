#ifndef GEO_NETWORK_CLIENT_CHECKTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_CHECKTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

class CheckTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<CheckTrustLineTransaction> Shared;

public:
    CheckTrustLineTransaction(
        const NodeUUID &nodeUUID,
        const SerializedEquivalent equivalent,
        const NodeUUID &contractorUUID,
        bool isActionInitiator,
        TrustLinesManager *manager,
        Logger &logger);

    CheckTrustLineTransaction(
        const NodeUUID &nodeUUID,
        const SerializedEquivalent equivalent,
        const NodeUUID &contractorUUID,
        ContractorID contractorID,
        bool isActionInitiator,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // log
    const string logHeader() const
    noexcept;

private:
    const NodeUUID mContractorUUID;
    ContractorID mContractorID;
    bool mIsActionInitiator;
    TrustLinesManager *mTrustLinesManager;
};


#endif //GEO_NETWORK_CLIENT_CHECKTRUSTLINETRANSACTION_H
