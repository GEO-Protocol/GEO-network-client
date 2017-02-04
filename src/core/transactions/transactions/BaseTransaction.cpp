#include "BaseTransaction.h"

BaseTransaction::BaseTransaction(
        BaseTransaction::TransactionType type,
        NodeUUID &nodeUUID) :

        mType(type),
        mNodeUUID(nodeUUID){}

BaseTransaction::BaseTransaction() {}

signals::connection BaseTransaction::addOnMessageSendSlot(
    const SendMessageSignal::slot_type &slot) const {

    return outgoingMessageIsReadySignal.connect(slot);
}

const BaseTransaction::TransactionType BaseTransaction::transactionType() const {

    return mType;
}

const NodeUUID &BaseTransaction::nodeUUID() const {

    return mNodeUUID;
}

const TransactionUUID &BaseTransaction::UUID() const {

    return mTransactionUUID;
}

void BaseTransaction::setContext(
    Message::Shared message) {

    mContext = message;
}

pair<ConstBytesShared, size_t> BaseTransaction::serializeContext() {
    return mContext->serialize();
}

void BaseTransaction::addMessage(
    Message::Shared message,
    const NodeUUID &nodeUUID) {

    outgoingMessageIsReadySignal(
        message,
        nodeUUID
    );
}

void BaseTransaction::increaseStepsCounter() {

    mStep += 1;
}

void BaseTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
}

void BaseTransaction::resetRequestsCounter() {

    mRequestCounter = 0;
}

pair<BytesShared, size_t> BaseTransaction::serializeParentToBytes() {

    size_t bytesCount = sizeof(uint16_t) +
        NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        sizeof(uint16_t) +
        sizeof(uint16_t);
    byte *data = (byte *) calloc(
        bytesCount,
        sizeof(byte)
    );
    //-----------------------------------------------------
    uint16_t transactionType = mType;
    memcpy(
      data,
      &transactionType,
      sizeof(uint16_t)
    );
    //-----------------------------------------------------
    memcpy(
        data + sizeof(uint16_t),
        mNodeUUID.data,
        NodeUUID::kBytesSize
    );
    //-----------------------------------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    //-----------------------------------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        &mStep,
        sizeof(uint16_t)
    );
    //-----------------------------------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + sizeof(uint16_t),
        &mRequestCounter,
        sizeof(uint16_t)
    );
    //-----------------------------------------------------
    return make_pair(
        BytesShared(data, free),
        bytesCount
    );
}

void BaseTransaction::deserializeParentFromBytes(
    BytesShared buffer) {

    uint16_t *transactionType = new (buffer.get()) uint16_t;
    mType = (TransactionType) *transactionType;
    //-----------------------------------------------------
    memcpy(
        mNodeUUID.data,
        buffer.get() + sizeof(uint16_t),
        NodeUUID::kBytesSize
    );
    //-----------------------------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer.get() + sizeof(uint16_t) + NodeUUID::kBytesSize,
        TransactionUUID::kBytesSize
    );
    //-----------------------------------------------------
    uint16_t *step = new (buffer.get() + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize) uint16_t;
    mStep = *step;
    //-----------------------------------------------------
    uint16_t *requestCounter = new (buffer.get() + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + sizeof(uint16_t)) uint16_t;
    mStep = *requestCounter;
}

const size_t BaseTransaction::kOffsetToDataBytes() {

    return sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + sizeof(uint16_t) + sizeof(uint16_t);
}

TransactionResult::Shared BaseTransaction::transactionResultFromCommand(
    CommandResult::Shared result) {

    TransactionResult *transactionResult = new TransactionResult();
    transactionResult->setCommandResult(CommandResult::Shared(result));
    return TransactionResult::Shared(transactionResult);
}
