#include "CoordinatorReservationRequestMessage.h"


CoordinatorReservationRequestMessage::CoordinatorReservationRequestMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const vector<pair<PathUUID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
    const NodeUUID& nextNodeInThePath) :

    FinalAmountsConfigurationMessage(
        senderUUID,
        transactionUUID,
        finalAmountsConfig),
    mNextPathNode(nextNodeInThePath)
{}

CoordinatorReservationRequestMessage::CoordinatorReservationRequestMessage(
    BytesShared buffer) :

    FinalAmountsConfigurationMessage(buffer)
{
    auto parentMessageOffset = FinalAmountsConfigurationMessage::kOffsetToInheritedBytes();
    auto nextNodeUUIDOffset = buffer.get() + parentMessageOffset;

    memcpy(
        mNextPathNode.data,
        nextNodeUUIDOffset,
        mNextPathNode.kBytesSize);
}


const NodeUUID& CoordinatorReservationRequestMessage::nextNodeInPathUUID() const
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
    auto parentBytesAndCount = FinalAmountsConfigurationMessage::serializeToBytes();
    size_t totalBytesCount =
        + parentBytesAndCount.second
        + mNextPathNode.kBytesSize;

    BytesShared buffer = tryMalloc(totalBytesCount);
    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    auto nextPathNodeOffset =
        initialOffset + parentBytesAndCount.second;

    memcpy(
        nextPathNodeOffset,
        mNextPathNode.data,
        mNextPathNode.kBytesSize);

    return make_pair(
        buffer,
        totalBytesCount);
}
