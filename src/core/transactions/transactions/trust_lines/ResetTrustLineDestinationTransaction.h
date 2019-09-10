#ifndef GEO_NETWORK_CLIENT_RESETTRUSTLINEDESTINATIONTRANSACTION_H
#define GEO_NETWORK_CLIENT_RESETTRUSTLINEDESTINATIONTRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../network/messages/trust_lines/TrustLineResetMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../contractors/ContractorsManager.h"


class ResetTrustLineDestinationTransaction : public BaseTransaction {

public:
    typedef shared_ptr<ResetTrustLineDestinationTransaction> Shared;

public:
    ResetTrustLineDestinationTransaction(
        TrustLineResetMessage::Shared message,
        ContractorsManager *contractorsManager,
        TrustLinesManager *manager,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

private:
    TransactionResult::SharedConst sendTrustLineErrorConfirmation(
        ConfirmationMessage::OperationState errorState);

protected:
    static const uint32_t kWaitMillisecondsForResponse = 20000;
    static const uint16_t kMaxCountSendingAttempts = 3;

private:
    ContractorID mContractorID;
    TrustLineResetMessage::Shared mMessage;
    ContractorsManager *mContractorsManager;
    TrustLinesManager *mTrustLinesManager;
};

#endif //GEO_NETWORK_CLIENT_RESETTRUSTLINEDESTINATIONTRANSACTION_H
