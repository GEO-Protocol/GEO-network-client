#include "GatewayNotificationReceiverTransaction.h"

GatewayNotificationReceiverTransaction::GatewayNotificationReceiverTransaction(
    GatewayNotificationMessage::Shared message,
    ContractorsManager *contractorsManager,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    StorageHandler *storageHandler,
    Logger &logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::GatewayNotificationReceiverType,
        message->transactionUUID(),
        message->equivalent(),
        logger),
    mMessage(message),
    mContractorsManager(contractorsManager),
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
                mMessage->idOnReceiverSide)) {
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
                mMessage->idOnReceiverSide,
                isContractorGateway);
        } catch (IOError &e) {
            ioTransaction->rollback();
            warning() << "Attempt to set contractor " << mMessage->idOnReceiverSide << " as gateway failed. "
                      << "IO transaction can't be completed. "
                      << "Details are: " << e.what();
            sendMessage<ConfirmationMessage>(
                mMessage->idOnReceiverSide,
                mEquivalent,
                mContractorsManager->idOnContractorSide(mMessage->idOnReceiverSide),
                mMessage->transactionUUID(),
                ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
            return resultDone();
        }
    }

    if (!isContractorNeighbor) {
        warning() << "Sender " << mMessage->idOnReceiverSide << " is not neighbor of current node on all equivalents";
        sendMessage<ConfirmationMessage>(
            mMessage->idOnReceiverSide,
            mEquivalent,
            mContractorsManager->idOnContractorSide(mMessage->idOnReceiverSide),
            mMessage->transactionUUID(),
            ConfirmationMessage::ErrorShouldBeRemovedFromQueue);
        return resultDone();
    }

    vector<pair<SerializedEquivalent, vector<BaseAddress::Shared>>> neighborsAddressesByEquivalents;
    for (const auto &equivalent : mEquivalentsSubsystemsRouter->equivalents()) {
        // if node is gateway it not share by routing tables, because they are very large
        if (mEquivalentsSubsystemsRouter->iAmGateway(equivalent)) {
            continue;
        }
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent);
        if (trustLinesManager->trustLineIsPresent(mMessage->idOnReceiverSide)) {
            neighborsAddressesByEquivalents.emplace_back(
                equivalent,
                getNeighborsForEquivalent(equivalent));
        }
    }
    debug() << "Send routing tables to node " << mMessage->idOnReceiverSide;
    sendMessage<RoutingTableResponseMessage>(
        mMessage->idOnReceiverSide,
        mContractorsManager->idOnContractorSide(mMessage->idOnReceiverSide),
        mTransactionUUID,
        neighborsAddressesByEquivalents);
    return resultDone();
}

vector<BaseAddress::Shared> GatewayNotificationReceiverTransaction::getNeighborsForEquivalent(
    const SerializedEquivalent equivalent) const
{
    auto neighbors = mEquivalentsSubsystemsRouter->trustLinesManager(equivalent)->firstLevelNeighbors();
    vector<BaseAddress::Shared> result;
    for(auto &node:neighbors){
        if(node == mMessage->idOnReceiverSide)
            continue;
        result.push_back(
            mContractorsManager->contractorMainAddress(
                node));
    }
    return result;
}

const string GatewayNotificationReceiverTransaction::logHeader() const
{
    stringstream s;
    s << "[GatewayNotificationReceiverTA: " << currentTransactionUUID() << " " << mEquivalent << "]";
    return s.str();
}
