#include "ReservationsInRelationToNodeMessage.h"

ReservationsInRelationToNodeMessage::ReservationsInRelationToNodeMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations) :

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mReservations(reservations)
{}

ReservationsInRelationToNodeMessage::ReservationsInRelationToNodeMessage(
    BytesShared buffer):
    TransactionMessage(buffer)
{
    auto parentMessageOffset = TransactionMessage::kOffsetToInheritedBytes();
    auto bytesBufferOffset = buffer.get() + parentMessageOffset;
    //----------------------------------------------------
    RecordCount *reservationsCount = new (bytesBufferOffset) RecordCount;
    bytesBufferOffset += sizeof(RecordCount);
    //-----------------------------------------------------
    mReservations.reserve(*reservationsCount);
    for (RecordNumber idx = 0; idx < *reservationsCount; idx++) {

        // PathID
        PathID *pathID = new (bytesBufferOffset) PathID;
        bytesBufferOffset += sizeof(PathID);

        // Amount
        vector<byte> bufferTrustLineAmount(
            bytesBufferOffset,
            bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        TrustLineAmount reservationAmount = bytesToTrustLineAmount(bufferTrustLineAmount);

        // Direction
        // todo : add type ReservationDirectionSize
        uint8_t *direction = new (bytesBufferOffset)uint8_t;
        bytesBufferOffset += sizeof(uint8_t);
        auto reservationEnumDirection = static_cast<AmountReservation::ReservationDirection>(*direction);

        auto amountReservation = make_shared<AmountReservation>(
            NodeUUID::empty(),
            reservationAmount,
            reservationEnumDirection);
        mReservations.push_back(
            make_pair(
                *pathID,
                amountReservation));
    }
}

const vector<pair<Message::PathID, AmountReservation::ConstShared>>& ReservationsInRelationToNodeMessage::reservations() const
{
    return mReservations;
}

pair<BytesShared, size_t> ReservationsInRelationToNodeMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount =
        + parentBytesAndCount.second
        + sizeof(RecordCount)
        + mReservations.size() *
          (sizeof(PathID) + kTrustLineAmountBytesCount + sizeof(uint8_t));

    BytesShared buffer = tryMalloc(bytesCount);

    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    auto bytesBufferOffset = initialOffset + parentBytesAndCount.second;
    //----------------------------------------------------
    RecordCount reservationsCount = (RecordCount)mReservations.size();
    memcpy(
        bytesBufferOffset,
        &reservationsCount,
        sizeof(RecordCount));
    bytesBufferOffset += sizeof(RecordCount);
    //----------------------------------------------------
    for (auto const &it : mReservations) {
        memcpy(
            bytesBufferOffset,
            &it.first,
            sizeof(PathID));
        bytesBufferOffset += sizeof(PathID);

        vector<byte> serializedAmount = trustLineAmountToBytes(it.second->amount());
        memcpy(
            bytesBufferOffset,
            serializedAmount.data(),
            kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;

        const auto kDirection = it.second->direction();
        memcpy(
            bytesBufferOffset,
            &kDirection,
            sizeof(uint8_t));
        bytesBufferOffset += sizeof(uint8_t);
    }
    //----------------------------------------------------
    return make_pair(
        buffer,
        bytesCount);
}

const Message::MessageType ReservationsInRelationToNodeMessage::typeID() const
{
    return Message::Payments_ReservationsInRelationToNode;
}
