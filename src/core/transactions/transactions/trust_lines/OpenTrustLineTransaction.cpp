#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    InitTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    bool iAmGateway,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::OpenTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mContractorUUID(mCommand->contractorUUID()),
    mCountSendingAttempts(0),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mIAmGateway(iAmGateway)
{
    mStep = Initialization;
}

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    const SerializedEquivalent equivalent,
    const NodeUUID &contractorUUID,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    bool iAmGateway,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::OpenTrustLineTransaction,
        nodeUUID,
        equivalent,
        logger),
    mContractorUUID(contractorUUID),
    mCountSendingAttempts(0),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mIAmGateway(iAmGateway)
{
    mStep = NextAttempt;
}

TransactionResult::SharedConst OpenTrustLineTransaction::run()
{
    info() << "step " << mStep;
    switch (mStep) {
        case Stages::Initialization: {
            return runInitializationStage();
        }
        case Stages::NextAttempt: {
            return runNextAttemptStage();
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
    info() << "Try init TL to " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    if (mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line already present.";
        return resultProtocolError();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        mTrustLines->open(
            mContractorUUID,
            ioTransaction);
        info() << "TrustLine to the node " << mContractorUUID
               << " successfully initialised.";

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceInitializationStage(
            BaseTransaction::OpenTrustLineTransaction);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceInitializationStage(
            BaseTransaction::OpenTrustLineTransaction);
#endif

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->removeTrustLine(
            mContractorUUID);
        error() << "Error during saving TA. Details: " << e.what();
        return resultUnexpectedError();
    }

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    sendMessage<TrustLineInitialMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mIAmGateway);
    mCountSendingAttempts++;
    info() << "Message with TL opening request was sent";

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runNextAttemptStage()
{
    info() << "runNextAttemptStage";
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultDone();
    }
    info() << "Try init TL to " << mContractorUUID;

    if (mContractorUUID == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultDone();
    }

    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        warning() << "Trust line is absent.";
        return resultDone();
    }

    if (mTrustLines->trustLineState(mContractorUUID) != TrustLine::Init) {
        warning() << "Invalid TL state " << mTrustLines->trustLineState(mContractorUUID);
        return resultDone();
    }

#ifdef TESTS
    mTrustLinesInfluenceController->testThrowExceptionOnSourceResumingStage(
        BaseTransaction::OpenTrustLineTransaction);
    mTrustLinesInfluenceController->testTerminateProcessOnSourceResumingStage(
        BaseTransaction::OpenTrustLineTransaction);
#endif

    sendMessage<TrustLineInitialMessage>(
        mContractorUUID,
        mEquivalent,
        mNodeUUID,
        mTransactionUUID,
        mContractorUUID,
        mIAmGateway);
    mCountSendingAttempts++;
    info() << "Message with TL opening request was sent";
    processPongMessage(mContractorUUID);

    mStep = ResponseProcessing;
    return resultOK();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response.";
        if (mCountSendingAttempts < kMaxCountSendingAttempts) {
            sendMessage<TrustLineInitialMessage>(
                mContractorUUID,
                mEquivalent,
                mNodeUUID,
                mTransactionUUID,
                mContractorUUID,
                mIAmGateway);
            mCountSendingAttempts++;
            info() << "Send message " << mCountSendingAttempts << " times";
            return resultWaitForMessageTypes(
                {Message::TrustLines_Confirmation},
                kWaitMillisecondsForResponse);
        }
        info() << "Transaction will be closed and send ping";
        sendMessage<PingMessage>(
            mContractorUUID,
            0,
            mNodeUUID);
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " send response on opening TL. gateway: " << message->isContractorGateway();
    if (message->senderUUID != mContractorUUID) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(mContractorUUID)) {
        error() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept opening TL. Response code: " << message->state();
        mTrustLines->removeTrustLine(
            mContractorUUID,
            ioTransaction);
        return resultDone();
    }

    try {
        mTrustLines->setTrustLineState(
            mContractorUUID,
            TrustLine::Active,
            ioTransaction);
        if (message->isContractorGateway()) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                mContractorUUID,
                true);
        }

#ifdef TESTS
        mTrustLinesInfluenceController->testThrowExceptionOnSourceProcessingResponseStage(
            BaseTransaction::OpenTrustLineTransaction);
        mTrustLinesInfluenceController->testTerminateProcessOnSourceProcessingResponseStage(
            BaseTransaction::OpenTrustLineTransaction);
#endif

        populateHistory(
            ioTransaction,
            TrustLineRecord::Opening);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Attempt to process confirmation from contractor " << mContractorUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        throw e;
    }

    publicKeysSharingSignal(
        mContractorUUID, mEquivalent);
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
        mContractorUUID,
        0);

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}