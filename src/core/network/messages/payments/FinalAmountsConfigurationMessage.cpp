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
    mPaymentNodesIds(paymentNodesIds),
    mIsReceiptContains(false)
{}

FinalAmountsConfigurationMessage::FinalAmountsConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
    const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
    const TrustLineAmount &amount,
    const KeyNumber publicKeyNumber,
    const lamport::Signature::Shared signature) :

    RequestMessageWithReservations(
        equivalent,
        senderUUID,
        transactionUUID,
        finalAmountsConfig),
    mPaymentNodesIds(paymentNodesIds),
    mIsReceiptContains(true),
    mAmount(amount),
    mPublicKeyNumber(publicKeyNumber),
    mSignature(signature)
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
    memcpy(
        &mIsReceiptContains,
        bytesBufferOffset,
        sizeof(byte));
    //----------------------------------------------------
    if (mIsReceiptContains) {
        bytesBufferOffset += sizeof(byte);
        vector<byte> amountBytes(
            bytesBufferOffset,
            bytesBufferOffset + kTrustLineAmountBytesCount);
        mAmount = bytesToTrustLineAmount(amountBytes);
        bytesBufferOffset += kTrustLineAmountBytesCount;

        memcpy(
            &mPublicKeyNumber,
            bytesBufferOffset,
            sizeof(KeyNumber));
        bytesBufferOffset += sizeof(KeyNumber);

        auto signature = make_shared<lamport::Signature>(
            bytesBufferOffset);
        mSignature = signature;
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

bool FinalAmountsConfigurationMessage::isReceiptContains() const
{
    return mIsReceiptContains;
}

const TrustLineAmount& FinalAmountsConfigurationMessage::amount() const
{
    return mAmount;
}

const KeyNumber FinalAmountsConfigurationMessage::publicKeyNumber() const
{
    return mPublicKeyNumber;
}

const lamport::Signature::Shared FinalAmountsConfigurationMessage::signature() const
{
    return mSignature;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> FinalAmountsConfigurationMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = RequestMessageWithReservations::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + mPaymentNodesIds.size() *
                (NodeUUID::kBytesSize + sizeof(PaymentNodeID))
            + sizeof(byte);
    if (mIsReceiptContains) {
        bytesCount +=
                kTrustLineAmountBytesCount
                + sizeof(KeyNumber)
                + lamport::Signature::signatureSize();
    }

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
    memcpy(
        bytesBufferOffset,
        &mIsReceiptContains,
        sizeof(byte));
    //----------------------------------------------------
    if (mIsReceiptContains) {
        bytesBufferOffset += sizeof(byte);
        auto serializedAmount = trustLineAmountToBytes(mAmount);
        memcpy(
            bytesBufferOffset,
            serializedAmount.data(),
            kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;

        memcpy(
            bytesBufferOffset,
            &mPublicKeyNumber,
            sizeof(KeyNumber));
        bytesBufferOffset += sizeof(KeyNumber);

        memcpy(
            bytesBufferOffset,
            mSignature->data(),
            mSignature->signatureSize());
    }
    //----------------------------------------------------
    return make_pair(
        buffer,
        bytesCount);
}
