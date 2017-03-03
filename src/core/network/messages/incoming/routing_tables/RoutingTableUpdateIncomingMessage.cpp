#include "RoutingTableUpdateIncomingMessage.h"

RoutingTableUpdateIncomingMessage::RoutingTableUpdateIncomingMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType RoutingTableUpdateIncomingMessage::typeID() const {

    Message::MessageTypeID::RoutingTableUpdateIncomingMessageType;
}

const NodeUUID& RoutingTableUpdateIncomingMessage::initiatorUUID() const {

    return mInitiatorUUID;
}

const NodeUUID& RoutingTableUpdateIncomingMessage::contractorUUID() const {

    return mContractorUUID;
}

const TrustLineDirection RoutingTableUpdateIncomingMessage::direction() const {

    return mDirection;
}

pair<BytesShared, size_t> RoutingTableUpdateIncomingMessage::serializeToBytes() {

    throw NotImplementedError("RoutingTableUpdateIncomingMessage::serializeToBytes: "
                                  "Method not implemented.");
}

void RoutingTableUpdateIncomingMessage::deserializeFromBytes(
    BytesShared buffer) {

    SenderMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();

    memcpy(
        mInitiatorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;

    SerializedTrustLineDirection *direction = new (buffer.get() + bytesBufferOffset) SerializedTrustLineDirection;
    mDirection = (TrustLineDirection) *direction;
}
