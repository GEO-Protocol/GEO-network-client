#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../network/messages/trust_lines/TrustLineInitialMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"

#include "../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../subsystems_controller/SubsystemsController.h"
#include "../../../subsystems_controller/TrustLinesInfluenceController.h"

class AcceptTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

public:
    AcceptTrustLineTransaction(
        const NodeUUID &nodeUUID,
        TrustLineInitialMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        bool iAmGateway,
        SubsystemsController *subsystemsController,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

    const string logHeader() const
    noexcept;

private:
    TransactionResult::SharedConst sendTrustLineErrorConfirmation(
        ConfirmationMessage::OperationState errorState);

public:
    mutable PublicKeysSharingSignal publicKeysSharingSignal;

protected:
    NodeUUID mContractorUUID;

    TrustLinesManager *mTrustLinesManager;
    StorageHandler *mStorageHandler;

    SubsystemsController *mSubsystemsController;
    TrustLinesInfluenceController *mTrustLinesInfluenceController;

    bool mIAmGateway;
    bool mSenderIsGateway;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
