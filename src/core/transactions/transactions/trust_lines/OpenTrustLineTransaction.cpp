#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    bool iAmGateway,
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
    mKeysStore(keystore),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst OpenTrustLineTransaction::run() {
    switch (mStep) {
        case Stages::Initialisation: {
            return runInitialisationStage();
        }
        case Stages::ResponseProcessing: {
            return runResponseProcessingStage();
        }
        default:
            throw ValueError(logHeader() + "::run: "
                "wrong value of mStep");
    }
}

TransactionResult::SharedConst OpenTrustLineTransaction::runInitialisationStage()
{
    if (!mSubsystemsController->isRunTrustLineTransactions()) {
        debug() << "It is forbidden run trust line transactions";
        return resultForbiddenRun();
    }
    const auto kContractor = mCommand->contractorUUID();
    info() << "Try open TL to " << kContractor;

    if (kContractor == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    if (mTrustLines->trustLineIsPresent(kContractor)) {
        warning() << "Trust line already present.";
        return resultProtocolError();
    }
    if (mCommand->amount() == 0) {
        warning() << "Can't establish trust line with zero amount.";
        return resultProtocolError();
    }

    mTrustLines->open(
        kContractor,
        mCommand->amount());
    info() << "Outgoing trust line to the node " << kContractor
           << " successfully initialised with " << mCommand->amount();
    // todo serialize transaction

    // Notifying remote node about trust line state changed.
    // Network communicator knows, that this message must be forced to be delivered,
    // so the TA itself might finish without any response from the remote node.
    if (mIAmGateway) {
        sendMessage<SetIncomingTrustLineFromGatewayMessage>(
            mCommand->contractorUUID(),
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            mCommand->contractorUUID(),
            mCommand->amount());
    } else {
        sendMessage<SetIncomingTrustLineMessage>(
            mCommand->contractorUUID(),
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            mCommand->contractorUUID(),
            mCommand->amount());
    }

        mStep = ResponseProcessing;
        return resultOK();
}

TransactionResult::SharedConst OpenTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response";
        // todo serializing of this TA need discuss
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " confirmed opening TL. gateway: " << message->gateway();
    if (message->senderUUID != mCommand->contractorUUID()) {
        warning() << "Sender is not contractor of this transaction";
        return resultContinuePreviousState();
    }
    if (!mTrustLines->trustLineIsPresent(message->senderUUID)) {
        warning() << "Something wrong, because TL must be created";
        // todo : need correct reaction
        return resultDone();
    }
    auto ioTransaction = mStorageHandler->beginTransaction();
    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept opening TL. Response code: " << message->state();
        mTrustLines->removeTrustLine(
            message->senderUUID);
        processConfirmationMessage(message);
        return resultDone();
    }
    try {
        // save into storage, because open create TL only in memory
        mTrustLines->save(
            ioTransaction,
            message->senderUUID);
        mTrustLines->setTrustLineState(
            ioTransaction,
            message->senderUUID,
            TrustLine::KeysPending);
        if (message->gateway()) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                message->senderUUID,
                true);
        }
        populateHistory(
            ioTransaction,
            TrustLineRecord::Opening);
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Attempt to process confirmation from contractor " << message->senderUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        return resultDone();
    }

    processConfirmationMessage(message);
    const auto kTransaction = make_shared<PublicKeysSharingSourceTransaction>(
        mNodeUUID,
        message->senderUUID,
        mEquivalent,
        mTrustLines,
        mStorageHandler,
        mKeysStore,
        mLog);
    launchSubsidiaryTransaction(kTransaction);
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
        mCommand->amount());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}