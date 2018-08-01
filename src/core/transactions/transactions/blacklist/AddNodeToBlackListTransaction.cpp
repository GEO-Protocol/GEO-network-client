#include "AddNodeToBlackListTransaction.h"

AddNodeToBlackListTransaction::AddNodeToBlackListTransaction(
    NodeUUID &nodeUUID,
    AddNodeToBlackListCommand::Shared command,
    StorageHandler *storageHandler,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController,
    Keystore *keystore,
    Logger &logger):
    BaseTransaction(
        BaseTransaction::AddNodeToBlackListTransactionType,
        nodeUUID,
        0,      //none equivalent
        logger),
    mCommand(command),
    mStorageHandler(storageHandler),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),
    mKeysStore(keystore)
{}

TransactionResult::SharedConst AddNodeToBlackListTransaction::run()
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

    for (const auto equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLineManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);

        if (!trustLineManager->trustLineIsPresent(kContractor)) {
            continue;
        }

        if (trustLineManager->incomingTrustAmount(kContractor) == TrustLine::kZeroAmount()) {
            continue;
        }

        const auto kTransaction = make_shared<CloseIncomingTrustLineTransaction>(
            mNodeUUID,
            equivalent,
            kContractor,
            trustLineManager,
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(equivalent),
            mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        launchSubsidiaryTransaction(kTransaction);
        info() << "CloseIncomingTrustLineTransaction with " << kContractor
               << " on equivalent " << equivalent << " successfully launched.";
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        ioTransaction->blackListHandler()->addNode(
            kContractor);
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