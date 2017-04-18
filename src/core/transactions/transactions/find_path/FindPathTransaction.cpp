#include "FindPathTransaction.h"

FindPathTransaction::FindPathTransaction(
    NodeUUID &nodeUUID,
    const NodeUUID &contractorUUID,
    const TransactionUUID &requestedTransactionUUID,
    PathsManager *pathsManager,
    ResourcesManager *resourcesManager,
    Logger *logger) :

    BaseTransaction(
        BaseTransaction::TransactionType::FindPathTransactionType,
        nodeUUID,
        logger),

    mContractorUUID(contractorUUID),
    mRequestedTransactionUUID(requestedTransactionUUID),
    mPathsManager(pathsManager),
    mResourcesManager(resourcesManager),
    mRequestCounter(0) {}

TransactionResult::SharedConst FindPathTransaction::run() {

    info() << "run\t" << currentTransactionUUID() << " I am " << mNodeUUID;

    if (!mContext.empty()) {
        return checkTransactionContext();

    } else {
        if (mRequestCounter < kMaxRequestsCount) {
            sendMessageToRemoteNode();
            increaseRequestsCounter();

        } else {
            mPathsManager->findPathsOnSelfArea(
                mContractorUUID);
            mResourcesManager->putResource(
                make_shared<PathsResource>(
                    mRequestedTransactionUUID,
                    mPathsManager->pathCollection()));
            return make_shared<const TransactionResult>(
                TransactionState::exit());
        }
    }
    return waitingForResponseState();

}

TransactionResult::SharedConst FindPathTransaction::checkTransactionContext() {

    info() << "context size\t" << mContext.size();
    if (mContext.size() > 0) {
        if (mContext.size() == previousContextSize) {
            return buildPaths();
        }
        previousContextSize = mContext.size();
        return waitingForResponseState();

    } else {
        throw ConflictError("FindPathTransaction::checkTransactionContext: "
                                    "Unexpected context size.");
    }
}

TransactionResult::SharedConst FindPathTransaction::buildPaths() {

    for (auto itResponseMessage = mContext.begin(); itResponseMessage != mContext.end(); ++itResponseMessage) {

        auto responseMessage = *itResponseMessage;
        if (responseMessage->typeID() == Message::MessageType::Paths_ResultRoutingTableFirstLevel) {
            ResultRoutingTable1LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable1LevelMessage>(
                responseMessage);

            mRT1 = response->rt1();
            info() << "receive RT1, size: " << mRT1.size();
            isReceiveContractorRT1 = true;
        }

        if (responseMessage->typeID() == Message::MessageType::Paths_ResultRoutingTableSecondLevel) {
            ResultRoutingTable2LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable2LevelMessage>(
                responseMessage);

            mRT2.insert(response->rt2().begin(), response->rt2().end());
            info() << "receive RT2, size: " << mRT2.size();
            isReceiveContractorRT1 = true;
        }

        if (responseMessage->typeID() == Message::MessageType::Paths_ResultRoutingTableThirdLevel) {
            ResultRoutingTable3LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable3LevelMessage>(
                responseMessage);

            mRT3.insert(response->rt3().begin(), response->rt3().end());
            info() << "receive RT3, size: " << mRT3.size();
            isReceiveContractorRT1 = true;
        }

    }
    mPathsManager->findPaths(
        mContractorUUID,
        mRT1,
        mRT2,
        mRT3);
    // TODO : remove after testing
    /*mPathsManager->findPathsTest(
        mContractorUUID,
        mRT1,
        mRT2,
        mRT3);*/
    mResourcesManager->putResource(
        make_shared<PathsResource>(
            mRequestedTransactionUUID,
            mPathsManager->pathCollection()));
    return make_shared<const TransactionResult>(
        TransactionState::exit());
}

void FindPathTransaction::sendMessageToRemoteNode() {

    isReceiveContractorRT1 = false;
    previousContextSize = 0;
    mRT1.clear();
    mRT2.clear();
    mRT3.clear();
    info() << "sendMessageToRemoteNode\t" << mContractorUUID;
    sendMessage<RequestRoutingTablesMessage>(
        mContractorUUID,
        mNodeUUID,
        currentTransactionUUID());
}

TransactionResult::SharedConst FindPathTransaction::waitingForResponseState() {

    info() << "waitingForResponseState";
    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageType::Paths_ResultRoutingTableFirstLevel,
             Message::MessageType::Paths_ResultRoutingTableSecondLevel,
             Message::MessageType::Paths_ResultRoutingTableThirdLevel},
            kConnectionTimeout));
}

void FindPathTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
    info() << "increaseRequestsCounter\t" << mRequestCounter;
}

const string FindPathTransaction::logHeader() const
{
    stringstream s;
    s << "[FindPathTA]";

    return s.str();
}
