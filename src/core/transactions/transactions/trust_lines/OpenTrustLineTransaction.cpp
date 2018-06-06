#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    SubsystemsController *subsystemsController,
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

    if (kContractor == mNodeUUID) {
        warning() << "Attempt to launch transaction against itself was prevented.";
        return resultProtocolError();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();

    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        auto kOperationResult = mTrustLines->setOutgoing(
            ioTransaction,
            kContractor,
            mCommand->amount());
        // todo check kOperationResult
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

    } catch (ValueError &) {
        ioTransaction->rollback();
        warning() << "Attempt to set outgoing trust line to the node " << kContractor << " failed. "
                  << "Cannot open trustline with zero amount.";
        return resultProtocolError();

    } catch (IOError &e) {
        ioTransaction->rollback();

        // remove created TL from TrustLinesManager
        mTrustLines->trustLines().erase(mCommand->contractorUUID());
        warning() << "Attempt to open outgoing trust line to the node " << kContractor << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst OpenTrustLineTransaction::runResponseProcessingStage()
{
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " confirmed opening TL. gateway: " << message->gateway();
    auto ioTransaction = mStorageHandler->beginTransaction();
    if (message->state() != ConfirmationMessage::OK) {
        warning() << "Contractor didn't accept opening TL. Response code: " << message->state();
        mTrustLines->removeTrustLine(
            ioTransaction,
            message->senderUUID);
        return resultDone();
    }
    try {
        if (message->gateway()) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                message->senderUUID,
                true);
        }
        mTrustLines->setTrustLineState(
            ioTransaction,
            message->senderUUID,
            TrustLine::KeysPending);
        populateHistory(
            ioTransaction,
            TrustLineRecord::Opening);
    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        error() << "Attempt to process confirmation from contractor " << message->senderUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        return resultDone();
    } catch (IOError &e) {
        ioTransaction->rollback();
        error() << "Attempt to process confirmation from contractor " << message->senderUUID << " failed. "
                << "Trust Line not found. Details are: " << e.what();
        return resultDone();
    }

    processConfirmationMessage(message);
    const auto kTransaction = make_shared<PublicKeysSharingSourceTransaction>(
        mNodeUUID,
        message->senderUUID,
        mEquivalent,
        mTrustLines,
        mStorageHandler,
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