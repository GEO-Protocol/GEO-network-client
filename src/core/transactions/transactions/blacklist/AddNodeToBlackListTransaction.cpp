#include "AddNodeToBlackListTransaction.h"

AddNodeToBlackListTransaction::AddNodeToBlackListTransaction(
    NodeUUID &nodeUUID,
    AddNodeToBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    TrustLinesManager *trustLinesManager,
    SubsystemsController *subsystemsController,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::AddNodeToBlackListTransactionType,
        nodeUUID,
        logger),
    mCommand(command),
    mStorageHandler(storageHandler),
    mTrustLinesManager(trustLinesManager),
    mSubsystemsController(subsystemsController)
{}

AddNodeToBlackListCommand::Shared AddNodeToBlackListTransaction::command() const
{
    return AddNodeToBlackListCommand::Shared();
}

TransactionResult::SharedConst AddNodeToBlackListTransaction::run()
{
    auto ioTransaction = mStorageHandler->beginTransaction();
    const auto contractorNode = mCommand->contractorUUID();

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
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mTrustLinesManager->closeIncoming(
            ioTransaction,
            kContractor);

        populateHistory(ioTransaction);

        info() << "Incoming trust line from the node " << kContractor
               << " successfully closed.";

        // Notifying remote node about trust line state changed.
        // Network communicator knows, that this message must be forced to be delivered,
        // so the TA itself might finish without any response from the remote node.
        sendMessage<CloseOutgoingTrustLineMessage>(
            mCommand->contractorUUID(),
            mNodeUUID,
            mTransactionUUID);

        ioTransaction->blackListHandler()->addNode(contractorNode);
        info() << "Node " << kContractor << " successfully added to black list";
        return resultOK();

    } catch (NotFoundError &e) {
        info() << "There are no opened incoming trust line from the node " << kContractor;
        ioTransaction->blackListHandler()->addNode(contractorNode);
        info() << "Node " << kContractor << " successfully added to black list";
        return resultOK();

    } catch (IOError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to close incoming trust line from the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

void AddNodeToBlackListTransaction::populateHistory(
    IOTransaction::Shared ioTransaction)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        TrustLineRecord::ClosingIncoming,
        mCommand->contractorUUID());

    ioTransaction->historyStorage()->saveTrustLineRecord(record);
#endif
}

TransactionResult::SharedConst AddNodeToBlackListTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

TransactionResult::SharedConst AddNodeToBlackListTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst AddNodeToBlackListTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string AddNodeToBlackListTransaction::logHeader() const
{
    stringstream s;
    s << "[AddNodeToBlackListTA: " << currentTransactionUUID() << "] ";
    return s.str();
}