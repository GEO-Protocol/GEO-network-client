#include "RoutingTableUpdateIncomingMessage.h"

RoutingTableUpdateIncomingMessage::RoutingTableUpdateIncomingMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType RoutingTableUpdateIncomingMessage::typeID() const {

    return Message::MessageTypeID::RoutingTableUpdateIncomingMessageType;
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

const RoutingTableUpdateOutgoingMessage::UpdatingStep RoutingTableUpdateIncomingMessage::updatingStep() const {

    return mUpdatingStep;
}

pair<BytesShared, size_t> RoutingTableUpdateIncomingMessage::serializeToBytes() {

    throw NotImplementedError("RoutingTableUpdateIncomingMessage::serializeToBytes: "
                                  "Method not implemented.");
}

void RoutingTableUpdateIncomingMessage::deserializeFromBytes(
    BytesShared buffer) {

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();

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
    bytesBufferOffset += sizeof(SerializedTrustLineDirection);

    RoutingTableUpdateOutgoingMessage::SerializedUpdatingStep *step = new (buffer.get() + bytesBufferOffset) RoutingTableUpdateOutgoingMessage::SerializedUpdatingStep;
    mUpdatingStep = (RoutingTableUpdateOutgoingMessage::UpdatingStep) *step;
}