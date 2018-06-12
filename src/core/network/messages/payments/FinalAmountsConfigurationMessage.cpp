#include "FinalAmountsConfigurationMessage.h"

FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
    const map<NodeUUID, PaymentNodeID> &paymentNodesIds) :

    RequestMessageWithReservations(
        equivalent,
        senderUUID,
        transactionUUID,
        finalAmountsConfig),
    mPaymentNodesIds(paymentNodesIds)
{}

FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
    const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
    const vector<pair<PathID, AmountReservation::ConstShared>> &reservations) :

    RequestMessageWithReservations(
        equivalent,
        senderUUID,
        transactionUUID,
        finalAmountsConfig),
    mPaymentNodesIds(paymentNodesIds),
    mReservations(reservations)
{}

FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage(
    BytesShared buffer):
    RequestMessageWithReservations(buffer)
{
    auto parentMessageOffset = RequestMessageWithReservations::kOffsetToInheritedBytes();
    auto bytesBufferOffset = buffer.get() + parentMessageOffset;
    //----------------------------------------------------
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

const Message::MessageType FinalAmountsConfigurationMessage::typeID() const
{
    return Message::Payments_FinalAmountsConfiguration;
}

const map<NodeUUID, PaymentNodeID>& FinalAmountsConfigurationMessage::paymentNodesIds() const
{
    return mPaymentNodesIds;
}

const vector<pair<PathID, AmountReservation::ConstShared>>& FinalAmountsConfigurationMessage::reservations() const
{
    return mReservations;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> FinalAmountsConfigurationMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = RequestMessageWithReservations::serializeToBytes();
    size_t bytesCount =
            + parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + mPaymentNodesIds.size() *
                (NodeUUID::kBytesSize + sizeof(PaymentNodeID))
            + sizeof(SerializedRecordsCount)
            + mReservations.size() *
                (sizeof(PathID)
                 + kTrustLineAmountBytesCount
                 + sizeof(AmountReservation::SerializedReservationDirectionSize));

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
        bytesBufferOffset += NodeUUID::kBytesSize;

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
