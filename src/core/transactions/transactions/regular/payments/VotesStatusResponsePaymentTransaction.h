#ifndef GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
#define GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H

#include "../../base/BaseTransaction.h"
#include "../../../../contractors/ContractorsManager.h"
#include "../../../../io/storage/StorageHandler.h"
#include "../../../../network/messages/payments/VotesStatusRequestMessage.hpp"
#include "../../../../network/messages/payments/ParticipantsVotesMessage.h"
#include "../../../../crypto/lamportscheme.h"
#include "../../../../subsystems_controller/SubsystemsController.h"

using namespace crypto;

class VotesStatusResponsePaymentTransaction:
        public BaseTransaction{
public:
    VotesStatusResponsePaymentTransaction(
        VotesStatusRequestMessage::Shared message,
        ContractorsManager *contractorsManager,
        StorageHandler *storageHandler,
        bool isRequestedTransactionCurrentlyInProcessing,
        SubsystemsController *subsystemsController,
        Logger &logger);

    TransactionResult::SharedConst run() override;

protected:
    const string logHeader() const override;

protected:
    VotesStatusRequestMessage::Shared mRequest;
    ContractorsManager *mContractorsManager;
    StorageHandler *mStorageHandler;
    bool mIsRequestedTransactionCurrentlyInProcessing;
    SubsystemsController *mSubsystemsController;
};
#endif //GEO_NETWORK_CLIENT_VOUTESSTATUSRESPONSEPAYMENTTRANSACTION_H
