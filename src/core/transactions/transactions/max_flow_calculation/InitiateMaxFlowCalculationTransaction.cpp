#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationCommand::Shared command,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger *logger) :

        BaseTransaction(
                BaseTransaction::TransactionType::InitiateMaxFlowCalculationTransactionType,
                nodeUUID
        ),
        mCommand(command),
        mTrustLinesManager(manager),
        mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
        mLog(logger){}

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
        BytesShared buffer,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager) :

    mTrustLinesManager(manager),
    mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager){

    deserializeFromBytes(buffer);
}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> InitiateMaxFlowCalculationTransaction::serializeToBytes() const {

    auto parentBytesAndCount = BaseTransaction::serializeToBytes();
    auto commandBytesAndCount = mCommand->serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +  commandBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + parentBytesAndCount.second,
        commandBytesAndCount.first.get(),
        commandBytesAndCount.second
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void InitiateMaxFlowCalculationTransaction::deserializeFromBytes(
        BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    BytesShared commandBufferShared = tryCalloc(InitiateMaxFlowCalculationCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
            commandBufferShared.get(),
            buffer.get() + BaseTransaction::kOffsetToInheritedBytes(),
            InitiateMaxFlowCalculationCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    mCommand = InitiateMaxFlowCalculationCommand::Shared(
            new InitiateMaxFlowCalculationCommand(
                    commandBufferShared));
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::run() {

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "target: " + mCommand->contractorUUID().stringUUID());
    /*mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->outgoingFlows()) {
        {
            auto info = mLog->info("InitiateMaxFlowCalculationTransaction->run");
            info << "out flow " << nodeUUIDAndTrustLine.first.stringUUID() << "  " <<  nodeUUIDAndTrustLine.second << "\n";
        }
    }

    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));
    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->getIncomingFlows()) {
        mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "in flow: " + nodeUUIDAndTrustLine.first.stringUUID()
                                                                    + "  " + to_string((uint32_t)nodeUUIDAndTrustLine.second));
    }*/

    for (auto const &nodeUUIDAndTrustLine : mTrustLinesManager->getOutgoingFlows()) {
        mMaxFlowCalculationTrustLineManager->addTrustLine(
            make_shared<MaxFlowCalculationTrustLine>(
                mNodeUUID,
                nodeUUIDAndTrustLine.first,
                nodeUUIDAndTrustLine.second)
        );
        //mMaxFlowCalculationTrustLineManager->addFlow(mNodeUUID, nodeUUIDAndTrustLine.first, nodeUUIDAndTrustLine.second);
    }
    mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mvTrustLines.size()));
    /*for (const auto &it : mMaxFlowCalculationTrustLineManager->mEntities) {
        mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                      "key: " + ((NodeUUID)it.first).stringUUID());

        for (const auto &it1 : it.second->mIncomingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "inflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }

        for (const auto &it1 : it.second->mOutgoingFlows) {
            mLog->logInfo("ReceiveResultMaxFlowCalculationTransaction::run",
                          "outflow: " + ((NodeUUID) it1.first).stringUUID() + " "
                          + to_string((uint32_t)it1.second));
        }
    }*/

    sendMessageToRemoteNode();
    sendMessageOnFirstLevel();

    return waitingForResponseState();

}

void InitiateMaxFlowCalculationTransaction::sendMessageToRemoteNode() {

    Message *message = new InitiateMaxFlowCalculationMessage(
        mNodeUUID,
        mNodeUUID);

    addMessage(
        Message::Shared(message),
        mCommand->contractorUUID());
}

void InitiateMaxFlowCalculationTransaction::sendMessageOnFirstLevel() {

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithOutgoingFlow();
    for (auto const &it : outgoingFlowUuids) {
        Message *message = new SendMaxFlowCalculationSourceFstLevelMessage(
            mNodeUUID,
            mNodeUUID);

        mLog->logInfo("InitiateMaxFlowCalculationTransaction->sendFirst", ((NodeUUID)it).stringUUID());
        addMessage(
            Message::Shared(message),
            it);
    }

}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::waitingForResponseState() {

    /*TransactionState *transactionState = new TransactionState(
            microsecondsSinceGEOEpoch(
                    utc_now() + pt::microseconds(kConnectionTimeout * 1000)
            ),
            Message::MessageTypeID::ResponseMessageType,
            false
    );*/


    return make_shared<TransactionResult>(TransactionState::exit());
//    TransactionResult *transactionResult = new TransactionResult();
//    //transactionResult->setTransactionState(TransactionState::SharedConst(transactionState));
//    return TransactionResult::SharedConst(transactionResult);
}
