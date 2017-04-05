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
    if (mContext.size() == 1) {
        auto responseMessage = *mContext.begin();

        if (responseMessage->typeID() == Message::MessageTypeID::ResultRoutingTablesMessageType) {
            ResultRoutingTablesMessage::Shared response = static_pointer_cast<ResultRoutingTablesMessage>(
                responseMessage);

            vector<NodeUUID> rt1 = response->rt1();
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
            }

            mPathsManager->setContractorRoutingTables(response);
            mPathsManager->findPath();
            // TODO : remove after testing
            mPathsManager->findPathsTest();
        }

        mResourcesManager->putResource(
            make_shared<PathsResource>(
                mRequestedTransactionUUID,
                mPathsManager->pathCollection()));
        return make_shared<const TransactionResult>(
            TransactionState::exit());

    } else {
        throw ConflictError("FindPathTransaction::checkTransactionContext: "
                                    "Unexpected context size.");
    }
}

void FindPathTransaction::sendMessageToRemoteNode() {

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
            {Message::MessageTypeID::ResultRoutingTablesMessageType},
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
