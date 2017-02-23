#include "MaxFlowCalculationResultMessage.h"

MaxFlowCalculationResultMessage::MaxFlowCalculationResultMessage() {

}

MaxFlowCalculationResultMessage::MaxFlowCalculationResultMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID) :

    TransactionMessage(
        senderUUID, transactionUUID
    ) {}

pair<BytesShared, size_t> MaxFlowCalculationResultMessage::serializeToBytes() {

    return TransactionMessage::serializeToBytes();
}

void MaxFlowCalculationResultMessage::deserializeFromBytes(
    BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
}

const size_t MaxFlowCalculationResultMessage::kOffsetToInheritedBytes() {

    return TransactionMessage::kOffsetToInheritedBytes();
}
