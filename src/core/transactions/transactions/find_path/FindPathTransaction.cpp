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
    mResourcesManager(resourcesManager)
{}

TransactionResult::SharedConst FindPathTransaction::run()
{
#ifdef GETTING_PATHS_DEBUG_LOG
    info() << "run\t" << currentTransactionUUID() << " I am " << mNodeUUID;
    info() << "run\t" << "step: " << mStep;
    info() << "run\t" << "context size: " << mContext.size();
#endif
    switch (mStep) {
        case Stages::SendRequestForGettingRoutingTables:
            sendMessageToRemoteNode();
            mStep = Stages::BuildAllPaths;
            return waitingForResponseState();
        case Stages::BuildAllPaths:
            if (!mContext.empty()) {
                return buildPaths();
            } else {
                mPathsManager->findPathsOnSelfArea(
                    mContractorUUID);
                mResourcesManager->putResource(
                    make_shared<PathsResource>(
                        mRequestedTransactionUUID,
                        mPathsManager->pathCollection()));
                mStep = Stages::SendRequestForGettingRoutingTables;
                return make_shared<const TransactionResult>(
                    TransactionState::exit());
            }
        default:
            throw ValueError("FindPathTransaction::run: "
                                 "wrong value of mStep");
    }
}

TransactionResult::SharedConst FindPathTransaction::buildPaths()
{
    for (auto itResponseMessage = mContext.begin(); itResponseMessage != mContext.end(); ++itResponseMessage) {
        auto responseMessage = *itResponseMessage;
        if (responseMessage->typeID() == Message::MessageType::Paths_ResultRoutingTableFirstLevel) {
            ResultRoutingTable1LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable1LevelMessage>(
                responseMessage);
            mRT1 = response->rt1();
#ifdef GETTING_PATHS_DEBUG_LOG
            info() << "receive RT1, size: " << mRT1.size();
            for (auto &nodeUUID : mRT1) {
                info() << "\t" << nodeUUID;
            }
#endif
        }
        if (responseMessage->typeID() == Message::MessageType::Paths_ResultRoutingTableSecondLevel) {
            ResultRoutingTable2LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable2LevelMessage>(
                responseMessage);
            mRT2.insert(response->rt2().begin(), response->rt2().end());
//#ifdef GETTING_PATHS_DEBUG_LOG
            info() << "receive RT2, size: " << mRT2.size();
//            for (auto &nodeUUIDAndVect : mRT2) {
//                for (auto const &nodeUUID : nodeUUIDAndVect.second) {
//                    info() << "\t" << nodeUUID
//                           << "\t" << nodeUUIDAndVect.first;
//                }
//            }
//#endif
        }
        if (responseMessage->typeID() == Message::MessageType::Paths_ResultRoutingTableThirdLevel) {
            ResultRoutingTable3LevelMessage::Shared response = static_pointer_cast<ResultRoutingTable3LevelMessage>(
                responseMessage);
            mRT3.insert(response->rt3().begin(), response->rt3().end());
//#ifdef GETTING_PATHS_DEBUG_LOG
            info() << "receive RT3, size: " << mRT3.size();
//            for (auto &nodeUUIDAndVect : mRT3) {
//                for (auto const &nodeUUID : nodeUUIDAndVect.second) {
//                    info() << "\t" << nodeUUID
//                           << "\t" << nodeUUIDAndVect.first;
//                }
//            }
//#endif
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
    mPathsManager->clearPathsCollection();
    return make_shared<const TransactionResult>(
        TransactionState::exit());
}

void FindPathTransaction::sendMessageToRemoteNode()
{
    mRT1.clear();
    mRT2.clear();
    mRT3.clear();
#ifdef GETTING_PATHS_DEBUG_LOG
    info() << "sendMessageToRemoteNode\t" << mContractorUUID;
#endif
    sendMessage<RequestRoutingTablesMessage>(
        mContractorUUID,
        mNodeUUID,
        currentTransactionUUID());
}

TransactionResult::SharedConst FindPathTransaction::waitingForResponseState()
{
#ifdef GETTING_PATHS_DEBUG_LOG
    info() << "waitingForResponseState";
#endif
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypesAndAwakeAfterMilliseconds(
            {Message::MessageType::Paths_ResultRoutingTableFirstLevel,
             Message::MessageType::Paths_ResultRoutingTableSecondLevel,
             Message::MessageType::Paths_ResultRoutingTableThirdLevel},
            kConnectionTimeout));
}

const string FindPathTransaction::logHeader() const
{
    stringstream s;
    s << "[FindPathTA]";
    return s.str();
}
