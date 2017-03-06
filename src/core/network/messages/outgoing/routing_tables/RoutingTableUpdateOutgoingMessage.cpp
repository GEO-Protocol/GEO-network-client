#include "RoutingTableUpdateOutgoingMessage.h"

RoutingTableUpdateOutgoingMessage::RoutingTableUpdateOutgoingMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const NodeUUID &initiatorUUID,
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction,
    const UpdatingStep updatingStep) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mInitiatorUUID(initiatorUUID),
    mContractorUUID(contractorUUID) {

    mDirection = direction;
    mUpdatingStep = updatingStep;
}

const Message::MessageType RoutingTableUpdateOutgoingMessage::typeID() const {

    return Message::MessageTypeID::RoutingTableUpdateOutgoingMessageType;
}

pair<BytesShared, size_t> RoutingTableUpdateOutgoingMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t dataBytesCount = parentBytesAndCount.second
                            + NodeUUID::kBytesSize
                            + NodeUUID::kBytesSize
                            + sizeof(SerializedTrustLineDirection)
                            + sizeof(SerializedUpdatingStep);

    auto dataBytesBuffer = tryMalloc(dataBytesCount);

    memcpy(
      dataBytesBuffer.get(),
      parentBytesAndCount.first.get(),
      parentBytesAndCount.second);
    size_t bytesBufferOffset = parentBytesAndCount.second;

    memcpy(
      dataBytesBuffer.get() + bytesBufferOffset,
      mInitiatorUUID.data,
      NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        dataBytesBuffer.get() + bytesBufferOffset,
        mContractorUUID.data,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    SerializedTrustLineDirection direction = (SerializedTrustLineDirection) mDirection;
    memcpy(
        dataBytesBuffer.get() + bytesBufferOffset,
        &direction,
        sizeof(SerializedTrustLineDirection));
    bytesBufferOffset += sizeof(SerializedTrustLineDirection);

    SerializedUpdatingStep step = (SerializedUpdatingStep) mUpdatingStep;
    memcpy(
        dataBytesBuffer.get() + bytesBufferOffset,
        &step,
        sizeof(SerializedUpdatingStep));

    return make_pair(
        dataBytesBuffer,
        dataBytesCount);
}

void RoutingTableUpdateOutgoingMessage::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("RoutingTableUpdateOutgoingMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}