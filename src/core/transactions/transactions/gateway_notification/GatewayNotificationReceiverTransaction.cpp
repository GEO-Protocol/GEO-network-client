#include "GatewayNotificationReceiverTransaction.h"

GatewayNotificationReceiverTransaction::GatewayNotificationReceiverTransaction(
    const NodeUUID &nodeUUID,
    GatewayNotificationMessage::Shared message,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationReceiverType,
        message->transactionUUID(),
        nodeUUID,
        message->equivalent(),
        logger),
    mMessage(message),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst GatewayNotificationReceiverTransaction::run()
{
    bool isContractorNeighbor = false;
    auto ioTransaction = mStorageHandler->beginTransaction();
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);
        if (trustLinesManager->isNeighbor(
                mMessage->senderUUID)) {
            continue;
        }
        isContractorNeighbor = true;
        bool isContractorGateway = find(
            mMessage->gatewayEquivalents().begin(),
            mMessage->gatewayEquivalents().end(),
            equivalent) != mMessage->gatewayEquivalents().end();
        try {
            trustLinesManager->setContractorAsGateway(
                ioTransaction,
                mMessage->senderUUID,
                isContractorGateway);
        } catch (IOError &e) {
            ioTransaction->rollback();
            warning() << "Attempt to set contractor " << mMessage->senderUUID << " as gateway failed. "
                      << "IO transaction can't be completed. "
                      << "Details are: " << e.what();
            return resultDone();
        }
    }

    if (!isContractorNeighbor) {
        warning() << "Sender " << mMessage->senderUUID << " is not neighbor of current node on all equivalents";
        sendMessage<ConfirmationMessage>(
            mMessage->senderUUID,
            mEquivalent,
            currentNodeUUID(),
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    debug() << "Send confirmation to node " << mMessage->senderUUID;
    sendMessage<ConfirmationMessage>(
        mMessage->senderUUID,
        mEquivalent,
        currentNodeUUID(),
        mMessage->transactionUUID());
    return resultDone();
}

const string GatewayNotificationReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationReceiverTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
