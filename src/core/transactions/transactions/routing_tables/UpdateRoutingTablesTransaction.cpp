#include "UpdateRoutingTablesTransaction.h"

UpdateRoutingTablesTransaction::UpdateRoutingTablesTransaction(
    NodeUUID &nodeUUID,
    UpdateRoutingTablesCommand::Shared command,
    TrustLinesManager *manager,
    StorageHandler *storageHandler,
    Logger &logger)
noexcept :
    BaseTransaction(
        BaseTransaction::RoutingTables_UpdateRoutingTable,
        nodeUUID,
        logger),
    mCommand(command),
    mTrustLinesManager(manager),
    mStorageHandler(storageHandler),
    mNodesToUpdate({})
{}

UpdateRoutingTablesCommand::Shared UpdateRoutingTablesTransaction::command() const
{
    return mCommand;
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::run() {
    switch (mStep) {
        case askNeighborsForTopologyChangingStage:
            return askNeighborsForTopologyChanging();
        case checkFirstAndSecondLevelCRC32SumForNeighborStage:
            return checkFirstAndSecondCRC32rt2Sum();
        case checkFirstLevelCRC32SumForNeighborStage:
            return checkFirstCRC32rt2Sum();
        case checkSecondLevelCRC32SumForNeighborStage:
            return checkSecondCRC32rt2Sum();
        case UpdateRT2Stage:
            return updateRougtingTables();
        default:
            throw ValueError("UpdateRoutingTablesTransaction::run: "
                                 "unexpected Trust line state");
    }
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::checkFirstAndSecondCRC32rt2Sum()
{
    if(mContext.size() == 0){
        info() << "There are no neighbors online. Exit transaction. ";
        return resultDone();
    }
    vector<NodeUUID> NodesForNextCheck;
    while(mContext.size() > 0){
        auto responseMessage = popNextMessage<CRC32Rt2ResponseMessage>();
        const auto kContractor = responseMessage->senderUUID;
        const auto kStepCRC32Sum = mTrustLinesManager->crc32SumSecondAndThirdLevelForNeighbor(kContractor);
        debug() << "ContractorUUID" << kContractor << endl;
        debug() << "CRC32Sum calcululated on Initiator " << kStepCRC32Sum << endl;
        debug() << "CRC32Sum from message" << responseMessage->crc32Rt2Sum() << endl;
        if(kStepCRC32Sum != responseMessage->crc32Rt2Sum()){
            debug() << kContractor << "CRC32SUMS not equal";
            NodesForNextCheck.push_back(kContractor);
        }
    }
    if (NodesForNextCheck.size() == 0){
        info() << "There are no changes in neighbors. Exit Transaction.";
        return resultDone();
    }

    auto message = make_shared<CRC32Rt2RequestMessage>(
        mNodeUUID,
        currentTransactionUUID(),
        CRC32Rt2RequestMessage::CRC32SumType::FirstLevel);

    for(const auto kNodeUUID: NodesForNextCheck)
        sendMessage(
            kNodeUUID,
            message);

    mStep = Stages::checkFirstLevelCRC32SumForNeighborStage;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypesAndAwakeAfterMilliseconds(
            {Message::MessageType::RoutingTables_CRC32Rt2ResponseMessage},
            mkWaitingForResponseTime));
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::checkFirstCRC32rt2Sum()
{
    if(mContext.size() == 0){
        info() << "There are no neighbors online. Exit transaction. ";
        return resultDone();
    }
    vector<NodeUUID> ThirdLevelNodesForNextCheck;
    while(mContext.size() > 0){
        auto responseMessage = popNextMessage<CRC32Rt2ResponseMessage>();
        const auto kContractor = responseMessage->senderUUID;
        const auto kStepCRC32Sum = mTrustLinesManager->crc32SumSecondLevelForNeighbor(kContractor);
        debug() << "ContractorUUID" << kContractor << endl;
        debug() << "CRC32Sum calcululated on Initiator " << kStepCRC32Sum << endl;
        debug() << "CRC32Sum from message" << responseMessage->crc32Rt2Sum() << endl;
        if(kStepCRC32Sum != responseMessage->crc32Rt2Sum()){
            // it sims that first level node has new trustline.
            // save its id for update with others
            mNodesToUpdate.push_back(make_pair(kContractor, 0));
        } else {
            ThirdLevelNodesForNextCheck.push_back(kContractor);
        }
    }

    auto message = make_shared<CRC32Rt2RequestMessage>(
        mNodeUUID,
        currentTransactionUUID(),
        CRC32Rt2RequestMessage::CRC32SumType::FirstLevel);

    if(ThirdLevelNodesForNextCheck.size() == 0 and mNodesToUpdate.size() == 0){
        info() << "Nodes with changes are offline. Exit Transaction. ";
        return resultDone();
    }

    set<NodeUUID> NodeForCRC32request;
    auto ioTransaction = mStorageHandler->beginTransaction();
    for(const auto kNodeUUID: ThirdLevelNodesForNextCheck){
        auto steprt2 = ioTransaction->routingTablesHandler()->neighborsOfOnRT2(kNodeUUID);
        for(const auto kThirdLevelNodeUUID: steprt2){
            mPathMap.insert(make_pair(kThirdLevelNodeUUID, kNodeUUID));
        }
        NodeForCRC32request.insert(steprt2.begin(), steprt2.end());
    }
    for(const auto kNodeUUID: NodeForCRC32request){
        sendMessage(
            kNodeUUID,
            message);
    }
    mStep = Stages::checkSecondLevelCRC32SumForNeighborStage;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypesAndAwakeAfterMilliseconds(
            {Message::MessageType::RoutingTables_CRC32Rt2ResponseMessage},
            mkWaitingForResponseTime));
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::checkSecondCRC32rt2Sum()
{
    if(mContext.size() == 0){
        info() << "There are no third level neighbors with wich current node has desync. Exit transaction.";
        if (mNodesToUpdate.size() != 0 )
            return updateRougtingTables();
        return resultDone();
    }
    while(mContext.size() > 0) {
        auto responseMessage = popNextMessage<CRC32Rt2ResponseMessage>();
        const auto kContractor = responseMessage->senderUUID;
        const auto kFirstLevelPathNode = mPathMap[kContractor];
        const auto kStepCRC32Sum = mTrustLinesManager->crc32SumThirdLevelForNeighbor(kContractor, kFirstLevelPathNode);
        debug() << "ContractorUUID" << kContractor << endl;
        debug() << "CRC32Sum calcululated on Initiator " << kStepCRC32Sum << endl;
        debug() << "CRC32Sum from message" << responseMessage->crc32Rt2Sum() << endl;
        if (kStepCRC32Sum != responseMessage->crc32Rt2Sum()) {
            // it sims that first level node has new trustline.
            // save its id for update with others
            mNodesToUpdate.push_back(make_pair(kContractor, 1));
        }
    }
    stringstream ss;
    for(auto it = mPathMap.begin(); it != mPathMap.end(); ++it) {
        ss << it->second << ";";
    }
    if (mNodesToUpdate.size() != 0 )
        return updateRougtingTables();
    info() << "It seems that current node has more records in rt3 for ["
            << ss.str() << "] nodes than they have in rt2 than they have.";
    return resultDone();
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::updateRougtingTables()
{
    for(auto kNodeUUIDAndNodeLevel: mNodesToUpdate){
        auto transaction = make_shared<NeighborsCollectingTransaction>(
            mNodeUUID,
            kNodeUUIDAndNodeLevel.first,
            mNodeUUID,
            kNodeUUIDAndNodeLevel.second,
            mStorageHandler,
            mLog);
        launchSubsidiaryTransaction(transaction);
    }
    return transactionResultFromCommand(mCommand->responseOK());
}

TransactionResult::SharedConst UpdateRoutingTablesTransaction::askNeighborsForTopologyChanging() {
    // Ask firstLevel neighbors for their first and second level rt CRC32Sum.
    // For this node it will be second and third Level
    auto message = make_shared<CRC32Rt2RequestMessage>(
        mNodeUUID,
        currentTransactionUUID(),
        CRC32Rt2RequestMessage::CRC32SumType::FirstAndSecondLevel);

    for(const auto kNodeUUID: mTrustLinesManager->rt1())
        sendMessage(
            kNodeUUID,
            message);
    mStep = Stages::checkFirstAndSecondLevelCRC32SumForNeighborStage;
    return make_shared<TransactionResult>(
        TransactionState::waitForMessageTypesAndAwakeAfterMilliseconds(
            {Message::MessageType::RoutingTables_CRC32Rt2ResponseMessage},
            mkWaitingForResponseTime));
}

const string UpdateRoutingTablesTransaction::logHeader() const
{
    stringstream s;
    s << "[UpdateRoutingTablesTransactionTA: " << currentTransactionUUID() << "]";
    return s.str();
}