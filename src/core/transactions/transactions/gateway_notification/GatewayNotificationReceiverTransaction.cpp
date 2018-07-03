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
        if (!trustLinesManager->trustLineIsPresent(
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
            sendMessage<ConfirmationMessage>(
                mMessage->senderUUID,
                mEquivalent,
                mNodeUUID,
                mMessage->transactionUUID(),
                ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
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

    vector<pair<SerializedEquivalent, set<NodeUUID>>> neighborsByEquivalents;
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);
        if (trustLinesManager->trustLineIsPresent(mMessage->senderUUID)) {
            neighborsByEquivalents.emplace_back(
                equivalent,
                getNeighborsForEquivalent(equivalent));
        }
    }
    debug() << "Send routing tables to node " << mMessage->senderUUID;
    sendMessage<RoutingTableResponseMessage>(
        mMessage->senderUUID,
        mNodeUUID,
        currentTransactionUUID(),
        neighborsByEquivalents);
    return resultDone();
}

set<NodeUUID> GatewayNotificationReceiverTransaction::getNeighborsForEquivalent(
    const SerializedEquivalent equivalent) const
{
    auto neighbors = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent)->firstLevelNeighbors();
    set<NodeUUID> result;
    for(auto &node:neighbors){
        if(node == mMessage->senderUUID)
            continue;
        result.insert(node);
    }
    return result;
}

const string GatewayNotificationReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationReceiverTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
