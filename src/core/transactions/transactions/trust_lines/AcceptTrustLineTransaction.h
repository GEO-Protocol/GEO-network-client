#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "base/BaseTrustLineTransaction.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineInitialMessage.h"
#include "../../../subsystems_controller/SubsystemsController.h"

class AcceptTrustLineTransaction : public BaseTrustLineTransaction {

public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

public:
    AcceptTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineInitialMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        SubsystemsController *subsystemsController,
        Keystore *keystore,
        bool iAmGateway,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger)
    noexcept;

    AcceptTrustLineTransaction(
        BytesShared buffer,
        const NodeUUID &nodeUUID,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        Keystore *keystore,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger);

    TransactionResult::SharedConst run();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

protected: // log
    const string logHeader() const
    noexcept;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runReceiveFirstKeyStage();

    TransactionResult::SharedConst runReceiveNextKeyStage();

    TransactionResult::SharedConst runReceiveAuditStage();

    TransactionResult::SharedConst runRecoveryStage();

    pair<BytesShared, size_t> serializeToBytes() const;

protected:
    TrustLineAmount mAmount;
    SubsystemsController *mSubsystemsController;
    bool mIAmGateway;
    bool mSenderIsGateway;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
