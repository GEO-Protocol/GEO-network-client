//
// Created by mc on 14.02.17.
//

#include "InitiateMaxFlowCalculationTransaction.h"

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
        NodeUUID &nodeUUID,
        InitiateMaxFlowCalculationCommand::Shared command,
        TrustLinesManager *manager,
        MaxFlowCalculationTrustLineManager *maxFlowCalculationTrustLineManager,
        Logger *logger) :

        MaxFlowCalculationTransaction(
                BaseTransaction::TransactionType::InitiateMaxFlowCalculationTransactionType,
                nodeUUID
        ),
        mCommand(command),
        mTrustLinesManager(manager),
        mMaxFlowCalculationTrustLineManager(maxFlowCalculationTrustLineManager),
        mLog(logger){}

InitiateMaxFlowCalculationTransaction::InitiateMaxFlowCalculationTransaction(
        BytesShared buffer,
        TrustLinesManager *manager) :

    mTrustLinesManager(manager){

    deserializeFromBytes(buffer);
}

InitiateMaxFlowCalculationCommand::Shared InitiateMaxFlowCalculationTransaction::command() const {

    return mCommand;
}

pair<BytesShared, size_t> InitiateMaxFlowCalculationTransaction::serializeToBytes() const {

    auto parentBytesAndCount = MaxFlowCalculationTransaction::serializeToBytes();
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

    MaxFlowCalculationTransaction::deserializeFromBytes(buffer);
    BytesShared commandBufferShared = tryCalloc(InitiateMaxFlowCalculationCommand::kRequestedBufferSize());
    //-----------------------------------------------------
    memcpy(
            commandBufferShared.get(),
            buffer.get() + MaxFlowCalculationTransaction::kOffsetToDataBytes(),
            InitiateMaxFlowCalculationCommand::kRequestedBufferSize()
    );
    //-----------------------------------------------------
    mCommand = InitiateMaxFlowCalculationCommand::Shared(
            new InitiateMaxFlowCalculationCommand(
                    commandBufferShared
            )
    );
}

TransactionResult::SharedConst InitiateMaxFlowCalculationTransaction::run() {
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "initiator: " + mNodeUUID.stringUUID());
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run", "target: " + mCommand->contractorUUID().stringUUID());
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "OutgoingFlows: " + to_string(mTrustLinesManager->getOutgoingFlows().size()));
    mLog->logInfo("InitiateMaxFlowCalculationTransaction->run",
                  "IncomingFlows: " + to_string(mTrustLinesManager->getIncomingFlows().size()));
    mLog->logInfo("InitiateMaxFlowCalculationTransaction::run",
                  "trustLineMap size: " + to_string(mMaxFlowCalculationTrustLineManager->mvTrustLines.size()));
    sendMessageToRemoteNode();
    sendMessageOnFirstLevel();



    return waitingForResponseState();

}

void InitiateMaxFlowCalculationTransaction::sendMessageToRemoteNode() {

    Message *message = new InitiateMaxFlowCalculationMessage(
            mNodeUUID,
            mNodeUUID,
            mTransactionUUID
    );

    addMessage(
            Message::Shared(message),
            mCommand->contractorUUID()
    );
}

void InitiateMaxFlowCalculationTransaction::sendMessageOnFirstLevel() {

    vector<NodeUUID> outgoingFlowUuids = mTrustLinesManager->getFirstLevelNeighborsWithOutgoingFlow();
    for (auto const &it : outgoingFlowUuids) {
        Message *message = new SendMaxFlowCalculationSourceFstLevelMessage(
            mNodeUUID,
            mNodeUUID,
            mTransactionUUID
        );

        mLog->logInfo("InitiateMaxFlowCalculationTransaction->sendFirst", ((NodeUUID)it).stringUUID());
        addMessage(
            Message::Shared(message),
            it
        );
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
