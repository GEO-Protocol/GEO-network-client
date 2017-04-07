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

    info() << "run\t" << UUID() << " I am " << mNodeUUID;

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
        if (responseMessage->typeID() == Message::MessageTypeID::ResultRoutingTablesMessageType) {
            ResultRoutingTablesMessage::Shared response = static_pointer_cast<ResultRoutingTablesMessage>(
                responseMessage);

            /*vector<NodeUUID> rt1 = response->rt1();
            info() << "receive RT1 size: " << rt1.size();
            for (auto &nodeUUID : rt1) {
                info() << "\t" << nodeUUID;
            }
            info() << "receive RT2 size: " << response->rt2().size();
            for (auto &nodeUUIDAndVect : response->rt2()) {
                for (auto const &nodeUUID : nodeUUIDAndVect.second) {
                    info() << "\t" << nodeUUID
                           << "\t" << nodeUUIDAndVect.first;
                }
            }
            info() << "receive RT3 size: " << response->rt3().size();
            for (auto &nodeUUIDAndVect : response->rt3()) {
                for (auto const &nodeUUID : nodeUUIDAndVect.second) {
                    info() << "\t" << nodeUUID
                           << "\t" << nodeUUIDAndVect.first;
                }
            }*/

            mPathsManager->findPaths(
                mContractorUUID,
                response->rt1(),
                response->rt2(),
                response->rt3());
            // TODO : remove after testing
            mPathsManager->findPathsTest(
                mContractorUUID,
                response->rt1(),
                response->rt2(),
                response->rt3());
            mResourcesManager->putResource(
                make_shared<PathsResource>(
                    mRequestedTransactionUUID,
                    mPathsManager->pathCollection()));
            return make_shared<const TransactionResult>(
                TransactionState::exit());
        }

        if (responseMessage->typeID() == Message::MessageTypeID::ResultRoutingTable1LevelMessageType) {
            ResultRoutingTable1LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable1LevelMessage>(
                responseMessage);

            mRT1 = response->rt1();
            info() << "receive RT1, size: " << mRT1.size();
            isReceiveContractorRT1 = true;
        }

        if (responseMessage->typeID() == Message::MessageTypeID::ResultRoutingTable2LevelMessageType) {
            ResultRoutingTable2LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable2LevelMessage>(
                responseMessage);

            mRT2.insert(response->rt2().begin(), response->rt2().end());
            info() << "receive RT2, size: " << mRT2.size();
            isReceiveContractorRT1 = true;
        }

        if (responseMessage->typeID() == Message::MessageTypeID::ResultRoutingTable3LevelMessageType) {
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
        UUID());
}

TransactionResult::SharedConst FindPathTransaction::waitingForResponseState() {

    info() << "waitingForResponseState";
    return transactionResultFromState(
        TransactionState::waitForMessageTypes(
            {Message::MessageTypeID::ResultRoutingTablesMessageType,
             Message::MessageTypeID::ResultRoutingTable1LevelMessageType,
             Message::MessageTypeID::ResultRoutingTable2LevelMessageType,
             Message::MessageTypeID::ResultRoutingTable3LevelMessageType},
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
