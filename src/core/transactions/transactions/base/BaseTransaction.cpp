#include "BaseTransaction.h"

BaseTransaction::BaseTransaction() {}

BaseTransaction::BaseTransaction(
    BaseTransaction::TransactionType type) :

    mType(type){

}

BaseTransaction::BaseTransaction(
        BaseTransaction::TransactionType type,
        NodeUUID &nodeUUID) :

        mType(type),
        mNodeUUID(nodeUUID){}

void BaseTransaction::addMessage(
    Message::Shared message,
    const NodeUUID &nodeUUID) {

    outgoingMessageIsReadySignal(
        message,
        nodeUUID
    );
}

const BaseTransaction::TransactionType BaseTransaction::transactionType() const {

    return mType;
}

const TransactionUUID &BaseTransaction::UUID() const {

    return mTransactionUUID;
}

const NodeUUID &BaseTransaction::nodeUUID() const {

    return mNodeUUID;
}

void BaseTransaction::increaseStepsCounter() {

    mStep += 1;
}

void BaseTransaction::resetStepsCounter() {

    mStep = 1;
}

void BaseTransaction::setExpectationResponsesCounter(
    uint16_t count) {

    mExpectationResponsesCount = count;
}

void BaseTransaction::resetExpectationResponsesCounter() {

    mExpectationResponsesCount = 0;
}

void BaseTransaction::setContext(
    Message::Shared message) {

    mContext.push_back(message);
}

void BaseTransaction::clearContext() {

    mContext.clear();
}

pair<BytesShared, size_t> BaseTransaction::serializeToBytes() const {

    size_t bytesCount = sizeof(SerializedTransactionType) +
        NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        sizeof(uint16_t);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //-----------------------------------------------------
    uint16_t transactionType = mType;
    memcpy(
        dataBytesShared.get(),
        &transactionType,
        sizeof(SerializedTransactionType)
    );
    dataBytesOffset += sizeof(SerializedTransactionType);
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    dataBytesOffset += TransactionUUID::kBytesSize;
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        mNodeUUID.data,
        NodeUUID::kBytesSize
    );
    dataBytesOffset += NodeUUID::kBytesSize;
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mStep,
        sizeof(uint16_t)
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void BaseTransaction::deserializeFromBytes(
    BytesShared buffer) {

    size_t bytesBufferOffset = 0;

    SerializedTransactionType *transactionType = new (buffer.get()) SerializedTransactionType;
    mType = (TransactionType) *transactionType;
    bytesBufferOffset += sizeof(SerializedTransactionType);
    //-----------------------------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer.get() + bytesBufferOffset,
        TransactionUUID::kBytesSize
    );
    bytesBufferOffset += TransactionUUID::kBytesSize;
    //-----------------------------------------------------
    memcpy(
        mNodeUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    //-----------------------------------------------------
    uint16_t *step = new (buffer.get() + bytesBufferOffset) uint16_t;
    mStep = *step;
}

const size_t BaseTransaction::kOffsetToInheritedBytes() {

    static const size_t offset = sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + sizeof(uint16_t);
    return offset;
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromCommand(
    CommandResult::SharedConst result) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(result);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromMessage(
    MessageResult::SharedConst messageResult) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setMessageResult(messageResult);
    return TransactionResult::SharedConst(transactionResult);
}

TransactionResult::SharedConst BaseTransaction::transactionResultFromState(
    TransactionState::SharedConst state) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setTransactionState(state);
    return TransactionResult::SharedConst(transactionResult);
}
