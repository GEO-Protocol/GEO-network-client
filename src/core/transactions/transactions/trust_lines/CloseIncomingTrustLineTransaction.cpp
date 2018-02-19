#include "CloseIncomingTrustLineTransaction.h"

CloseIncomingTrustLineTransaction::CloseIncomingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    CloseIncomingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    MaxFlowCalculationCacheManager *maxFlowCalculationCacheManager,
    MaxFlowCalculationNodeCacheManager *maxFlowCalculationNodeCacheManager,
    SubsystemsController *subsystemsController,
    Logger &logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::CloseIncomingTrustLineTransaction,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mMaxFlowCalculationCacheManager(maxFlowCalculationCacheManager),
    mMaxFlowCalculationNodeCacheManager(maxFlowCalculationNodeCacheManager),
    mSubsystemsController(subsystemsController)
{}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::run()
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
    TrustLine::ConstShared previousTL = nullptr;
    try {
        previousTL = mTrustLines->trustLineReadOnly(mCommand->contractorUUID());
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLines->closeIncoming(
            ioTransaction,
            kContractor);

        populateHistory(ioTransaction, TrustLineRecord::ClosingIncoming);
        mMaxFlowCalculationCacheManager->resetInitiatorCache();
        mMaxFlowCalculationNodeCacheManager->clearCashes();
        info() << "Incoming trust line from the node " << kContractor
               << " successfully closed.";

        // Notifying remote node about trust line state changed.
        // Network communicator knows, that this message must be forced to be delivered,
        // so the TA itself might finish without any response from the remote node.
        sendMessage<CloseOutgoingTrustLineMessage>(
            mCommand->contractorUUID(),
            mNodeUUID,
            mTransactionUUID,
            mCommand->contractorUUID());

        return resultOK();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to close incoming trust line from the node " << kContractor << " failed. "
               << "Details are: " << e.what();
        return resultProtocolError();

    } catch (IOError &e) {
        ioTransaction->rollback();
        // return closed TL
        auto trustLine = make_shared<TrustLine>(
            mCommand->contractorUUID(),
            previousTL->incomingTrustAmount(),
            previousTL->outgoingTrustAmount(),
            previousTL->balance(),
            previousTL->isContractorGateway());
        mTrustLines->trustLines()[mCommand->contractorUUID()] = trustLine;
        warning() << "Attempt to close incoming trust line from the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst CloseIncomingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
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
    s << "[CloseIncomingTrustLineTA: " << currentTransactionUUID() << "]";
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

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
#endif
}
