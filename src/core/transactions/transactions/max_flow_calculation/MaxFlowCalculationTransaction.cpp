#include "MaxFlowCalculationTransaction.h"

MaxFlowCalculationTransaction::MaxFlowCalculationTransaction(
    const BaseTransaction::TransactionType type,
    const NodeUUID &nodeUUID) :

    BaseTransaction(
        type,
        nodeUUID) {}

MaxFlowCalculationTransaction::MaxFlowCalculationTransaction(
    const TransactionType type) :

    BaseTransaction(
        type) {}

pair<BytesShared, size_t> MaxFlowCalculationTransaction::serializeToBytes() const {

    auto parentBytesAndCount = BaseTransaction::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void MaxFlowCalculationTransaction::deserializeFromBytes(
    BytesShared buffer) {

    BaseTransaction::deserializeFromBytes(buffer);
}


const size_t MaxFlowCalculationTransaction::kOffsetToDataBytes() {

    static const size_t offset = BaseTransaction::kOffsetToInheritedBytes();
    return offset;
}
