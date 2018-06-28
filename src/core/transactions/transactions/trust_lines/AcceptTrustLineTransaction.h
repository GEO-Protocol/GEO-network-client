#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"
#include "../../../io/storage/StorageHandler.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineMessage.h"
#include "../../../network/messages/trust_lines/SetIncomingTrustLineFromGatewayMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"
#include "../../../subsystems_controller/SubsystemsController.h"

class AcceptTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<AcceptTrustLineTransaction> Shared;

public:
    AcceptTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        SubsystemsController *subsystemsController,
        bool iAmGateway,
        Logger &logger)
    noexcept;

    AcceptTrustLineTransaction(
        const NodeUUID &nodeUUID,
        SetIncomingTrustLineFromGatewayMessage::Shared message,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        SubsystemsController *subsystemsController,
        bool iAmGateway,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

protected: // log
    const string logHeader() const
    noexcept;

protected:
    SetIncomingTrustLineMessage::Shared mMessage;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;
    SubsystemsController *mSubsystemsController;
    bool mIAmGateway;
    bool mSenderIsGateway;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINETRANSACTION_H
