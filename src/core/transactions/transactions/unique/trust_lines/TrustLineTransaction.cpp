#include "TrustLineTransaction.h"

TrustLineTransaction::TrustLineTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID,
    Logger *logger) :

    BaseTransaction(
        type,
        nodeUUID,
        logger) {

    setExpectationResponsesCounter(kResponsesCount);
}

TrustLineTransaction::TrustLineTransaction(
    const BaseTransaction::TransactionType type,
    Logger *logger) :

    BaseTransaction(
        type,
        logger) {

    setExpectationResponsesCounter(kResponsesCount);
}

void TrustLineTransaction::increaseRequestsCounter() {

    mRequestCounter += 1;
}

void TrustLineTransaction::resetRequestsCounter() {

    mRequestCounter = 0;
}

pair<BytesShared, size_t> TrustLineTransaction::serializeToBytes() const {

    auto parentBytesAndCount = BaseTransaction::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +
                        sizeof(uint16_t);

    BytesShared dataBytesShared = tryMalloc(
        bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mRequestCounter,
        sizeof(uint16_t));
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

void TrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(
        buffer);

    size_t bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    //----------------------------------------------------
    uint16_t *requestsCounter = new (buffer.get() + bytesBufferOffset) uint16_t;
    mRequestCounter = *requestsCounter;
}

const size_t TrustLineTransaction::kOffsetToDataBytes() {

    const size_t offset = BaseTransaction::kOffsetToInheritedBytes()
                                 + sizeof(uint16_t);
    return offset;
}