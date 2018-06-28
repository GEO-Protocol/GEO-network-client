#include "SetOutgoingTrustLineTransaction.h"


SetOutgoingTrustLineTransaction::SetOutgoingTrustLineTransaction(
    const NodeUUID &nodeUUID,
    SetOutgoingTrustLineCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    TopologyCacheManager *topologyCacheManager,
    MaxFlowCacheManager *maxFlowCacheManager,
    SubsystemsController *subsystemsController,
    Keystore *keystore,
    VisualInterface *visualInterface,
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
    mKeysStore(keystore),
    mVisualInterface(visualInterface),
    mIAmGateway(iAmGateway)
{}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::run()
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

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runInitialisationStage()
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

    // Trust line must be updated in the internal storage.
    // Also, history record must be written about this operation.
    // Both writes must be done atomically, so the IO transaction is used.
    auto ioTransaction = mStorageHandler->beginTransaction();
    try {
        // note: io transaction would commit automatically on destructor call.
        // there is no need to call commit manually.
        mOperationResult = mTrustLines->setOutgoing(
            ioTransaction,
            kContractor,
            mCommand->amount());

        mTrustLines->setTrustLineState(
            ioTransaction,
            mCommand->contractorUUID(),
            TrustLine::AuditPending);

        mTopologyCacheManager->resetInitiatorCache();
        mMaxFlowCacheManager->clearCashes();

        switch (mOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                info() << "Outgoing trust line to the node " << kContractor
                       << " successfully initialised with " << mCommand->amount();
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Updated: {
                info() << "Outgoing trust line to the node " << kContractor
                       << " successfully set to " << mCommand->amount();
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Closed: {
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

        if (mSubsystemsController->isWriteVisualResults()) {
            if (mOperationResult == TrustLinesManager::TrustLineOperationResult::Opened) {
                stringstream s;
                s << VisualResult::OutgoingTrustLineOpen << kTokensSeparator
                  << microsecondsSinceUnixEpoch() << kTokensSeparator
                  << currentTransactionUUID() << kTokensSeparator
                  << mCommand->contractorUUID() << kCommandsSeparator;
                auto message = s.str();

                try {
                    mVisualInterface->writeResult(
                        message.c_str(),
                        message.size());
                } catch (IOError &e) {
                    error() << "SetOutgoingTrustLineTransaction: "
                            "Error occurred when visual result has accepted. Details: " << e.message();
                }
            }
            if (mOperationResult == TrustLinesManager::TrustLineOperationResult::Closed) {
                stringstream s;
                s << VisualResult::OutgoingTrustLineClose << kTokensSeparator
                  << microsecondsSinceUnixEpoch() << kTokensSeparator
                  << currentTransactionUUID() << kTokensSeparator
                  << mCommand->contractorUUID() << kCommandsSeparator;
                auto message = s.str();

                try {
                    mVisualInterface->writeResult(
                        message.c_str(),
                        message.size());
                } catch (IOError &e) {
                    error() << "SetOutgoingTrustLineTransaction: "
                            "Error occurred when visual result has accepted. Details: " << e.message();
                }
            }
        }

        mStep = ResponseProcessing;
        return resultOK();

    } catch (ValueError &) {
        warning() << "Attempt to set outgoing trust line to the node " << kContractor << " failed. "
                  << "Cannot open trustline with zero amount.";
        return resultProtocolError();

    } catch (IOError &e) {
        ioTransaction->rollback();
        mTrustLines->trustLines()[mCommand->contractorUUID()] = make_shared<TrustLine>(
            mCommand->contractorUUID(),
            mPreviousTL->trustLineID(),
            mPreviousTL->incomingTrustAmount(),
            mPreviousTL->outgoingTrustAmount(),
            mPreviousTL->balance(),
            mPreviousTL->isContractorGateway(),
            mPreviousTL->state(),
            mPreviousTL->currentAuditNumber());
        warning() << "Attempt to set outgoing trust line to the node " << kContractor << " failed. "
                  << "IO transaction can't be completed. "
                  << "Details are: " << e.what();

        // Rethrowing the exception,
        // because the TA can't finish properly and no result may be returned.
        throw e;
    }
}

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::runResponseProcessingStage()
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
    try {
        if (message->state() != ConfirmationMessage::OK) {
            warning() << "Contractor didn't accept opening TL. Response code: " << message->state();
            mTrustLines->setOutgoing(
                ioTransaction,
                message->senderUUID,
                mPreviousTL->outgoingTrustAmount());
            mTrustLines->setTrustLineState(
                ioTransaction,
                message->senderUUID,
                mPreviousTL->state());
            processConfirmationMessage(message);
            return resultDone();
        }

        if (message->gateway()) {
            mTrustLines->setContractorAsGateway(
                ioTransaction,
                message->senderUUID,
                true);
        }
        switch (mOperationResult) {
            case TrustLinesManager::TrustLineOperationResult::Opened: {
                populateHistory(ioTransaction, TrustLineRecord::Opening);
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Updated: {
                populateHistory(ioTransaction, TrustLineRecord::Setting);
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::Closed: {
                populateHistory(ioTransaction, TrustLineRecord::Closing);
                break;
            }

            case TrustLinesManager::TrustLineOperationResult::NoChanges: {
                populateHistory(ioTransaction, TrustLineRecord::Setting);
                break;
            }
        }
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

TransactionResult::SharedConst SetOutgoingTrustLineTransaction::resultOK()
{
    return transactionResultFromCommandAndWaitForMessageTypes(
        mCommand->responseOK(),
        {Message::TrustLines_Confirmation},
        kWaitMillisecondsForResponse);
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
