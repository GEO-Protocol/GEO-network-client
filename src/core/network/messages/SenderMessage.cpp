#include "SenderMessage.h"


SenderMessage::SenderMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    ContractorID idOnReceiverSide,
    vector<BaseAddress::Shared> senderAddresses)
    noexcept :
    EquivalentMessage(
        equivalent),
    senderUUID(senderUUID),
    idOnReceiverSide(idOnReceiverSide),
    senderAddresses(senderAddresses)
{}

SenderMessage::SenderMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    vector<BaseAddress::Shared> senderAddresses)
    noexcept :
    EquivalentMessage(
        equivalent),
    senderUUID(senderUUID),
    senderAddresses(senderAddresses)
{}

SenderMessage::SenderMessage(
    BytesShared buffer)
    noexcept:
    EquivalentMessage(buffer)
{
    auto bytesBufferOffset = EquivalentMessage::kOffsetToInheritedBytes();
    memcpy(
        const_cast<NodeUUID*>(&senderUUID),
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    memcpy(
        &idOnReceiverSide,
        buffer.get() + bytesBufferOffset,
        sizeof(ContractorID));
    bytesBufferOffset += sizeof(ContractorID);

    uint16_t senderAddressesCnt;
    memcpy(
        &senderAddressesCnt,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
    bytesBufferOffset += sizeof(byte);

    for (int idx = 0; idx < senderAddressesCnt; idx++) {
        const uint16_t kAddressType =
            *(reinterpret_cast<BaseAddress::SerializedType *>(buffer.get() + bytesBufferOffset));

        switch (kAddressType) {
            case BaseAddress::IPv4_IncludingPort: {
                auto ipv4WithPortAddress = make_shared<IPv4WithPortAddress>(
                    buffer.get() + bytesBufferOffset);
                senderAddresses.push_back(ipv4WithPortAddress);
                bytesBufferOffset += ipv4WithPortAddress->serializedSize();
                break;
            }
            default: {
                // todo : need correct reaction
            }
        }
    }
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> SenderMessage::serializeToBytes() const
    noexcept(false)
{
    BytesSerializer serializer;

    serializer.enqueue(EquivalentMessage::serializeToBytes());
    serializer.enqueue(senderUUID);
    serializer.copy(idOnReceiverSide);
    serializer.copy((byte)senderAddresses.size());
    for (const auto &address : senderAddresses) {
        serializer.enqueue(
            address->serializeToBytes(),
            address->serializedSize());
    }
    return serializer.collect();
}

const size_t SenderMessage::kOffsetToInheritedBytes() const
    noexcept
{
    auto kOffset =
        EquivalentMessage::kOffsetToInheritedBytes()
        + NodeUUID::kBytesSize
        + sizeof(ContractorID)
        + sizeof(byte);
    for (const auto &address : senderAddresses) {
        kOffset += address->serializedSize();
    }
    return kOffset;
}
