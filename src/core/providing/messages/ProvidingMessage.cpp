#include "ProvidingMessage.h"

ProvidingMessage::ProvidingMessage(
    ProviderParticipantID participantID):
    mParticipantID(participantID)
{}

BytesShared ProvidingMessage::serializeToBytes() const
{
    SerializedProtocolVersion kProtocolVersion = ProtocolVersion::Latest;
    const SerializedType kMessageType = typeID();
    const auto kMessageSize = (MessageSize)serializedSize();
    auto buffer = tryMalloc(kMessageSize);

    auto dataSize = kMessageSize - sizeof(MessageSize);
    memcpy(
        buffer.get(),
        &dataSize,
        sizeof(MessageSize));

    memcpy(
        buffer.get() + sizeof(MessageSize),
        &kProtocolVersion,
        sizeof(SerializedProtocolVersion));

    memcpy(
        buffer.get() + sizeof(MessageSize) + sizeof(SerializedProtocolVersion),
        &kMessageType,
        sizeof(SerializedType));

    return buffer;
}

size_t ProvidingMessage::serializedSize() const
{
    return sizeof(MessageSize) +
           sizeof(SerializedProtocolVersion) +
           sizeof(SerializedType);
}