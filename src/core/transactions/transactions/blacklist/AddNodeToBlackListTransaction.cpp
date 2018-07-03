#include "AddNodeToBlackListTransaction.h"

AddNodeToBlackListTransaction::AddNodeToBlackListTransaction(
    NodeUUID &nodeUUID,
    AddNodeToBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    SubsystemsController *subsystemsController,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::AddNodeToBlackListTransactionType,
        nodeUUID,
        0,      //none equivalent
        logger),
    mCommand(command),
    mStorageHandler(storageHandler),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
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

    // Trust line must be deleted from the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    for (const auto equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLineManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);

        try {
            trustLineManager->closeIncoming(
                kContractor);

            populateHistory(
                equivalent,
                ioTransaction);
        } catch (NotFoundError &e) {
            info() << "There are no opened incoming trust line from the node " << kContractor
                   << " on equivalent " << equivalent;
            continue;
        } catch (IOError &e) {
            ioTransaction->rollback();
            warning() << "Attempt to close incoming trust line from the node " << kContractor << " failed. "
                      << "IO transaction can't be completed. "
                      << "Details are: " << e.what();

            // Rethrowing the exception,
            // because the TA can't finish properly and no result may be returned.
            throw e;
        }

        info() << "Incoming trust line from the node " << kContractor
               << " on equivalent " << equivalent << " successfully closed.";

        // Notifying remote node about trust line closing.
        // Network communicator knows, that this message must be forced to be delivered.
        sendMessage<CloseOutgoingTrustLineMessage>(
            mCommand->contractorUUID(),
            equivalent,
            mNodeUUID,
            mTransactionUUID,
            mCommand->contractorUUID());
    }

    try {
        ioTransaction->blackListHandler()->addNode(
            contractorNode);
    } catch (IOError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to add node " << kContractor << " to black list failed. "
                  << "IO transaction can't be completed. Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
        info() << "Node " << kContractor << " successfully added to black list";
        return resultOK();
}

void AddNodeToBlackListTransaction::populateHistory(
    const SerializedEquivalent equivalent,
    IOTransaction::Shared ioTransaction)
{
#ifndef TESTS
    auto record = make_shared<TrustLineRecord>(
        mTransactionUUID,
        TrustLineRecord::ClosingIncoming,
        mCommand->contractorUUID());

    ioTransaction->historyStorage()->saveTrustLineRecord(
        record,
        equivalent);
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