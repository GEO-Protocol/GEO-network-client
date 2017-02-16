#include "TrustLineTransaction.h"

TrustLineTransaction::TrustLineTransaction(
    BaseTransaction::TransactionType type,
    NodeUUID &nodeUUID,
    TransactionsScheduler *scheduler) :

    UniqueTransaction(
        type,
        nodeUUID,
        scheduler) {

    setExpectationResponsesCounter(kResponsesCount);
}

TrustLineTransaction::TrustLineTransaction(
    TransactionsScheduler *scheduler) :

    UniqueTransaction(scheduler) {

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
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mRequestCounter,
        sizeof(uint16_t)
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void TrustLineTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = BaseTransaction::kOffsetToInheritedBytes();
    //----------------------------------------------------
    uint16_t *requestsCounter = new (buffer.get() + bytesBufferOffset) uint16_t;
    mRequestCounter = *requestsCounter;
}

const size_t TrustLineTransaction::kOffsetToDataBytes() {

    static const size_t offset = BaseTransaction::kOffsetToInheritedBytes() + sizeof(uint16_t);
    return offset;
}