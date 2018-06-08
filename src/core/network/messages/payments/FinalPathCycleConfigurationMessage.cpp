#include "FinalPathCycleConfigurationMessage.h"

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<NodeUUID, PaymentNodeID> &paymentNodesIds) :

    RequestCycleMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        amount),
    mPaymentNodesIds(paymentNodesIds)
{}

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations) :

    RequestCycleMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        amount),
    mPaymentNodesIds(paymentNodesIds),
    mReservations(reservations)
{}

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    BytesShared buffer):

RequestCycleMessage(buffer)
{
    auto parentMessageOffset = RequestCycleMessage::kOffsetToInheritedBytes();
    auto bytesBufferOffset = buffer.get() + parentMessageOffset;

    auto *paymentNodesIdsCount = new (bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    for (SerializedRecordNumber idx = 0; idx < *paymentNodesIdsCount; idx++) {
        NodeUUID nodeUUID(bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        auto *paymentNodeID = new (bytesBufferOffset) PaymentNodeID;
        bytesBufferOffset += sizeof(PaymentNodeID);
        //---------------------------------------------------
        mPaymentNodesIds.insert(
            make_pair(
                nodeUUID,
                *paymentNodeID));
    }
    //----------------------------------------------------
    auto *reservationsCount = new (bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    mReservations.reserve(*reservationsCount);
    for (SerializedRecordNumber idx = 0; idx < *reservationsCount; idx++) {

        // PathID
        auto *pathID = new (bytesBufferOffset) PathID;
        bytesBufferOffset += sizeof(PathID);

        // Amount
        vector<byte> bufferTrustLineAmount(
            bytesBufferOffset,
            bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        TrustLineAmount reservationAmount = bytesToTrustLineAmount(bufferTrustLineAmount);

        // Direction
        auto *direction =
                new (bytesBufferOffset)AmountReservation::SerializedReservationDirectionSize;
        bytesBufferOffset += sizeof(AmountReservation::SerializedReservationDirectionSize);
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

const Message::MessageType FinalPathCycleConfigurationMessage::typeID() const
{
    return Message::Payments_FinalPathCycleConfiguration;
}

const map<NodeUUID, PaymentNodeID>& FinalPathCycleConfigurationMessage::paymentNodesIds() const
{
    return mPaymentNodesIds;
}

const vector<pair<PathID, AmountReservation::ConstShared>>& FinalPathCycleConfigurationMessage::reservations() const
{
    return mReservations;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> FinalPathCycleConfigurationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = RequestCycleMessage::serializeToBytes();
    size_t bytesCount =
            + parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + mPaymentNodesIds.size() *
              (NodeUUID::kBytesSize + sizeof(PaymentNodeID));

    BytesShared buffer = tryMalloc(bytesCount);
    auto initialOffset = buffer.get();
    memcpy(
        initialOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    auto bytesBufferOffset = initialOffset + parentBytesAndCount.second;

    //----------------------------------------------------
    auto paymentNodesIdsCount = (SerializedRecordsCount)mPaymentNodesIds.size();
    memcpy(
        bytesBufferOffset,
        &paymentNodesIdsCount,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //----------------------------------------------------
    for (auto const &it : mPaymentNodesIds) {
        memcpy(
            bytesBufferOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        bytesBufferOffset += sizeof(NodeUUID::kBytesSize);

        memcpy(
            bytesBufferOffset,
            &it.second,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);
    }
    //----------------------------------------------------
    auto reservationsCount = (SerializedRecordsCount)mReservations.size();
    memcpy(
        bytesBufferOffset,
        &reservationsCount,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
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
            sizeof(AmountReservation::SerializedReservationDirectionSize));
        bytesBufferOffset += sizeof(AmountReservation::SerializedReservationDirectionSize);
    }
    //----------------------------------------------------
    return make_pair(
        buffer,
        bytesCount);
}
