#include "RoutingTableUpdateOutgoingMessage.h"

RoutingTableUpdateOutgoingMessage::RoutingTableUpdateOutgoingMessage(
    const NodeUUID &senderUUID,
    const NodeUUID &initiatorUUID,
    const NodeUUID &contractorUUID,
    const TrustLineDirection direction) :

    SenderMessage(senderUUID),
    mInitiatorUUID(initiatorUUID),
    mContractorUUID(contractorUUID) {

    mDirection = direction;
}

const Message::MessageType RoutingTableUpdateOutgoingMessage::typeID() const {

    Message::MessageTypeID::RoutingTableUpdateOutgoingMessageType;
}

const NodeUUID& RoutingTableUpdateOutgoingMessage::initiatorUUID() const {

    return mInitiatorUUID;
}

const NodeUUID& RoutingTableUpdateOutgoingMessage::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineDirection RoutingTableUpdateOutgoingMessage::direction() const {

    return mDirection;
}

pair<BytesShared, size_t> RoutingTableUpdateOutgoingMessage::serializeToBytes() {

    auto parentBytesAndCount = SenderMessage::serializeToBytes();

    size_t dataBytesCount = parentBytesAndCount.second
                            + NodeUUID::kBytesSize
                            + NodeUUID::kBytesSize
                            + sizeof(SerializedTrustLineDirection);

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

    return make_pair(
        dataBytesBuffer,
        dataBytesCount);
}

void RoutingTableUpdateOutgoingMessage::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("RoutingTableUpdateOutgoingMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}
