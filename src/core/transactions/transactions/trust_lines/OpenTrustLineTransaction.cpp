#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    InitTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    bool iAmGateway,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::OpenTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst OpenTrustLineTransaction::run()
{
    info() << "step " << mStep;
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep");
    }
}

TransactionResult::SharedConst OpenTrustLineTransaction::runInitializationStage()
{
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }
    info() << "Try open TL to " << mCommand->contractorUUID();

    if (mCommand->contractorUUID() == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    if (mTrustLines->trustLineIsPresent(mCommand->contractorUUID())) {
        warning() << "Trust line already present.";
        return resultProtocolError();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLines->open(
            mCommand->contractorUUID(),
            0,
            ioTransaction);
        info() << "TrustLine to the node " << mCommand->contractorUUID()
               << " successfully initialised.";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnTLModifyingStage();
        mTrustLinesInfluenceController->testTerminateProcessOnTLModifyingStage();
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->removeTrustLine(
            mCommand->contractorUUID());
        error() << "Error during saving TA. Details: " << e.what();
        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<TrustLineInitialMessage>(
        mCommand->contractorUUID(),
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mCommand->contractorUUID(),
        mIAmGateway);

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response. Transaction will be closed, and wait for message";
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " send response on opening TL. gateway: " << message->isContractorGateway();
    if (message->senderUUID != mCommand->contractorUUID()) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mCommand->contractorUUID())) {
        error() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }

    processConfirmationMessage(message);
    auto ioTransaction = mStorageHandler->beginTransaction();
    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept opening TL. Response code: " << message->state();
        mTrustLines->removeTrustLine(
            mCommand->contractorUUID(),
            ioTransaction);
        return resultDone();
    }

#ifdef TESTS
    mTrustLinesInfluenceController->testThrowExceptionOnTLProcessingResponseStage();
    mTrustLinesInfluenceController->testTerminateProcessOnTLProcessingResponseStage();
#endif

    try {
        mTrustLines->setTrustLineState(
            mCommand->contractorUUID(),
            TrustLine::Active,
            ioTransaction);
        if (message->isContractorGateway()) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                mCommand->contractorUUID(),
                true);
        }
        populateHistory(
            ioTransaction,
            TrustLineRecord::Opening);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Attempt to process confirmation from contractor " << mCommand->contractorUUID() << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    publicKeysSharingSignal(
        mCommand->contractorUUID(), mEquivalent);
    return resultDone();
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

TransactionResult::SharedConst OpenTrustLineTransaction::resultUnexpectedError()
{
    return transactionResultFromCommand(
        mCommand->responseUnexpectedError());
}

const string OpenTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[OpenTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void OpenTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mCommand->contractorUUID(),
        0);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}