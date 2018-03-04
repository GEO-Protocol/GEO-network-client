#include "SetOutgoingTrustLineTransaction.h"


SetOutgoingTrustLineTransaction::SetOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    bool iAmGateway,
    Logger &logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::SetOutgoingTrustLineTransaction,
        nodeUUID,
        command->equivalent(),
        logger),
    mCommand(command),
    mTrustLines(manager),
    mStorageHandler(storageHandler),
    mTopologyCacheManager(topologyCacheManager),
    mMaxFlowCacheManager(maxFlowCacheManager),
    mSubsystemsController(subsystemsController),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::run()
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
    } catch (NotFoundError &e) {
        // Nothing actions, because TL will be created
    }

    TrustLinesManager::TrustLineOperationResult kOperationResult;
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        kOperationResult = mTrustLines->setOutgoing(
            ioTransaction,
            kContractor,
            mCommand->amount());

        switch (kOperationResult) {
        case TrustLinesManager::TrustLineOperationResult::Opened: {
            populateHistory(ioTransaction, TrustLineRecord::Opening);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();

            info() << "Outgoing trust line to the node " << kContractor
                   << " successfully initialised with " << mCommand->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Updated: {
            populateHistory(ioTransaction, TrustLineRecord::Setting);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();

            info() << "Outgoing trust line to the node " << kContractor
                   << " successfully set to " << mCommand->amount();
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::Closed: {
            populateHistory(ioTransaction, TrustLineRecord::Closing);
            mTopologyCacheManager->resetInitiatorCache();
            mMaxFlowCacheManager->clearCashes();

            info() << "Outgoing trust line to the node " << kContractor
                   << " successfully closed.";
            break;
        }

        case TrustLinesManager::TrustLineOperationResult::NoChanges: {
            // Trust line was set to the same value as previously.
            // By the first look, new history record is redundant here,
            // but this transaction might be launched only by the user,
            // so, in case if new amount is the same - then user knows it,
            // and new history record must be written too.
            populateHistory(ioTransaction, TrustLineRecord::Updating);
            info() << "Outgoing trust line to the node " << kContractor
                   << " successfully set to " << mCommand->amount();
            break;
        }
        }

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

        return resultOK();

    } catch (ValueError &) {
        ioTransaction->rollback();
        warning() << "Attempt to set outgoing trust line to the node " << kContractor << " failed. "
               << "Cannot open trustline with zero amount.";
        return resultProtocolError();

    } catch (NotFoundError &e) {
        ioTransaction->rollback();
        warning() << "Attempt to update outgoing trust line to the node " << kContractor << " failed. "
               << "Details are: " << e.what();
        return resultProtocolError();

    } catch (IOError &e) {
        ioTransaction->rollback();
        switch (kOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                // remove created TL from TrustLinesManager
                mTrustLines->trustLines().erase(mCommand->contractorUUID());
                break;
            }
            case TrustLinesManager::TrustLineOperationResult::Updated: {
                // Change outgoing TL amount to previous value
                mTrustLines->trustLines()[mCommand->contractorUUID()]->setOutgoingTrustAmount(
                    previousTL->outgoingTrustAmount());
                break;
            }
            case TrustLinesManager::TrustLineOperationResult::Closed: {
                // return closed TL
                auto trustLine = make_shared<TrustLine>(
                    mCommand->contractorUUID(),
                    previousTL->incomingTrustAmount(),
                    previousTL->outgoingTrustAmount(),
                    previousTL->balance(),
                    false);
                mTrustLines->trustLines()[mCommand->contractorUUID()] = trustLine;
                break;
            }
        }
        warning() << "Attempt to set outgoing trust line to the node " << kContractor << " failed. "
               << "IO transaction can't be completed. "
               << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommand(
        mCommand->responseOK());
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultForbiddenRun()
{
    return transactionResultFromCommand(
        mCommand->responseForbiddenRunTransaction());
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultProtocolError()
{
    return transactionResultFromCommand(
        mCommand->responseProtocolError());
}

const string SetOutgoingTrustLineTransaction::logHeader() const
    noexcept
{
    stringstream s;
    s << "[SetOutgoingTrustLineTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}

void SetOutgoingTrustLineTransaction::populateHistory(
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
