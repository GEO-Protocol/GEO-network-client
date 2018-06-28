#include "CloseIncomingTrustLineTransaction.h"

CloseIncomingTrustLineTransaction::CloseIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseIncomingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyTrustLinesManager *topologyTrustLinesManager,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    Logger &logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::CloseIncomingTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mTopologyTrustLinesManager(topologyTrustLinesManager),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::run()
{
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

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runInitialisationStage()
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

    try {
        mPreviousTL = mTrustLines->trustLineReadOnly(mCommand->contractorUUID());
    } catch (NotFoundError &e) {
        warning() << "Attempt to change not existing TL";
        return resultProtocolError();
    }

    // Trust line must be created (or updated) in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->closeIncoming(
            ioTransaction,
            kContractor);

        mTrustLines->setTrustLineState(
            ioTransaction,
            mCommand->contractorUUID(),
            TrustLine::AuditPending);

        // remove this TL from Topology TrustLines Manager
        mTopologyTrustLinesManager->addTrustLine(
            make_shared<TopologyTrustLine>(
                mNodeUUID,
                kContractor,
                make_shared<const TrustLineAmount>(0)));
        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();
        info() << "Incoming trust line from the node " << kContractor
               << " successfully closed.";

        // Notifying remote node about trust line state changed.
        // Network communicator knows, that this message must be forced to be delivered,
        // so the TA itself might finish without any response from the remote node.
        sendMessage<CloseOutgoingTrustLineMessage>(
            mCommand->contractorUUID(),
            mEquivalent,
            mNodeUUID,
            mTransactionUUID,
            mCommand->contractorUUID());

        mStep = ResponseProcessing;
        return resultOK();

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        mTrustLines->trustLines()[mCommand->contractorUUID()] = make_shared<TrustLine>(
            mCommand->contractorUUID(),
            mPreviousTL->trustLineID(),
            mPreviousTL->incomingTrustAmount(),
            mPreviousTL->outgoingTrustAmount(),
            mPreviousTL->balance(),
            mPreviousTL->isContractorGateway(),
            mPreviousTL->state(),
            mPreviousTL->currentAuditNumber());
        warning() << "Attempt to close incoming trust line from the node " << kContractor << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::runResponseProcessingStage()
{
    if (mContext.empty()) {
        warning() << "Contractor don't send response";
        // todo serializing of this TA need discuss
        return resultDone();
    }
    auto message = popNextMessage<TrustLineConfirmationMessage>();
    info() << "contractor " << message->senderUUID << " confirmed closing incoming TL.";
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
    try {
        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept closing incoming TL. Response code: " << message->state();
            mTrustLines->setIncoming(
                ioTransaction,
                message->senderUUID,
                mPreviousTL->incomingTrustAmount());
            mTrustLines->setTrustLineState(
                ioTransaction,
                message->senderUUID,
                mPreviousTL->state());
            processConfirmationMessage(message);
            return resultDone();
        }

        populateHistory(ioTransaction, TrustLineRecord::ClosingIncoming);
    } catch (IOError &e) {
        ioTransaction->rollback();
        // todo : need return intermediate state of TL
        error() << "Attempt to process confirmation from contractor " << message->senderUUID << " failed. "
                << "IO transaction can't be completed. Details are: " << e.what();
        return resultDone();
    }

    processConfirmationMessage(message);
    const auto kTransaction = make_shared<AuditSourceTransaction>(
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

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string CloseIncomingTrustLineTransaction::logHeader() const
noexcept
{
    stringstream s;
    s << "[CloseIncomingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void CloseIncomingTrustLineTransaction::populateHistory(
    IOTransaction::Shared ioTransaction,
    TrustLineRecord::TrustLineOperationType operationType)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        operationType,
        mCommand->contractorUUID());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        mEquivalent);
#endif
}
