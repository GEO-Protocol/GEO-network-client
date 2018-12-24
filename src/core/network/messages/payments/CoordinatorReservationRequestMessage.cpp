#include "CoordinatorReservationRequestMessage.h"


CoordinatorReservationRequestMessage::CoordinatorReservationRequestMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> senderAddresses,
    const TransactionUUID& transactionUUID,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
    BaseAddress::Shared nextNodeInThePath) :

    RequestMessageWithReservations(
        equivalent,
        senderAddresses,
        transactionUUID,
        finalAmountsConfig),
    mNextPathNode(nextNodeInThePath)
{}

CoordinatorReservationRequestMessage::CoordinatorReservationRequestMessage(
    BytesShared buffer) :

    RequestMessageWithReservations(buffer)
{
    size_t parentMessageOffset = RequestMessageWithReservations::kOffsetToInheritedBytes();

    mNextPathNode = deserializeAddress(buffer.get() + parentMessageOffset);
}

BaseAddress::Shared CoordinatorReservationRequestMessage::nextNodeInPath() const
{
    return mNextPathNode;
}

const Message::MessageType CoordinatorReservationRequestMessage::typeID() const
{
    return Message::Payments_CoordinatorReservationRequest;
}

/**
 * @throws bad_alloc;
 */
pair<BytesShared, size_t> CoordinatorReservationRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{    
    auto parentBytesAndCount = RequestMessageWithReservations::serializeToBytes();
    size_t totalBytesCount =
        + parentBytesAndCount.second
        + mNextPathNode->serializedSize();

    BytesShared buffer = tryMalloc(totalBytesCount);
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto serializedAddress = mNextPathNode->serializeToBytes();
    memcpy(
        buffer.get() + parentBytesAndCount.second,
        serializedAddress.get(),
        mNextPathNode->serializedSize());

    return make_pair(
        buffer,
        totalBytesCount);
}
