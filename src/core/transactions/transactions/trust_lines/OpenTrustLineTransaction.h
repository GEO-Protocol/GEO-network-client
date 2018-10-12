#ifndef GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
#define GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H

#include "../base/BaseTransaction.h"
#include "../../../interface/commands_interface/commands/trust_lines/InitTrustLineCommand.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../subsystems_controller/TrustLinesInfluenceController.h"
#include "../../../subsystems_controller/SubsystemsController.h"

#include "../../../network/messages/trust_lines/TrustLineInitialMessage.h"
#include "../../../network/messages/trust_lines/TrustLineConfirmationMessage.h"

class OpenTrustLineTransaction : public BaseTransaction {

public:
    typedef shared_ptr<OpenTrustLineTransaction> Shared;

public:
    OpenTrustLineTransaction(
        const NodeUUID &nodeUUID,
        InitTrustLineCommand::Shared command,
        TrustLinesManager *manager,
        StorageHandler *storageHandler,
        bool iAmGateway,
        SubsystemsController *subsystemsController,
        TrustLinesInfluenceController *trustLinesInfluenceController,
        Logger &logger)
    noexcept;

    TransactionResult::SharedConst run();

protected:
    enum Stages {
        Initialization = 1,
        ResponseProcessing = 2,
    };

protected:
    TransactionResult::SharedConst resultOK();

    TransactionResult::SharedConst resultForbiddenRun();

    TransactionResult::SharedConst resultProtocolError();

    TransactionResult::SharedConst resultUnexpectedError();

protected: // trust lines history shortcuts
    void populateHistory(
        IOTransaction::Shared ioTransaction,
        TrustLineRecord::TrustLineOperationType operationType);

    const string logHeader() const
    noexcept;

private:
    TransactionResult::SharedConst runInitializationStage();

    TransactionResult::SharedConst runResponseProcessingStage();

public:
    mutable PublicKeysSharingSignal publicKeysSharingSignal;

protected:
    static const uint32_t kWaitMillisecondsForResponse = 60000;

protected:
    InitTrustLineCommand::Shared mCommand;
    TrustLinesManager *mTrustLines;
    StorageHandler *mStorageHandler;

    TrustLinesInfluenceController *mTrustLinesInfluenceController;
    SubsystemsController *mSubsystemsController;
    bool mIAmGateway;
};


#endif //GEO_NETWORK_CLIENT_OPENTRUSTLINETRANSACTION_H
