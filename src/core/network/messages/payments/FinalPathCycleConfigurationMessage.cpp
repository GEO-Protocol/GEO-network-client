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
    mPaymentNodesIds(paymentNodesIds),
    mIsReceiptContains(false)
{}

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<NodeUUID, PaymentNodeID> &paymentNodesIds,
    const KeyNumber publicKeyNumber,
    const lamport::Signature::Shared signature) :

    RequestCycleMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        amount),
    mPaymentNodesIds(paymentNodesIds),
    mIsReceiptContains(true),
    mPublicKeyNumber(publicKeyNumber),
    mSignature(signature)
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
    memcpy(
        &mIsReceiptContains,
        bytesBufferOffset,
        sizeof(byte));
    //----------------------------------------------------
    if (mIsReceiptContains) {
        bytesBufferOffset += sizeof(byte);
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

const Message::MessageType FinalPathCycleConfigurationMessage::typeID() const
{
    return Message::Payments_FinalPathCycleConfiguration;
}

const map<NodeUUID, PaymentNodeID>& FinalPathCycleConfigurationMessage::paymentNodesIds() const
{
    return mPaymentNodesIds;
}

bool FinalPathCycleConfigurationMessage::isReceiptContains() const
{
    return mIsReceiptContains;
}

const KeyNumber FinalPathCycleConfigurationMessage::publicKeyNumber() const
{
    return mPublicKeyNumber;
}

const lamport::Signature::Shared FinalPathCycleConfigurationMessage::signature() const
{
    return mSignature;
}

/*!
 *
 * Throws bad_alloc;
 */
pair<BytesShared, size_t> FinalPathCycleConfigurationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = RequestCycleMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + mPaymentNodesIds.size() *
              (NodeUUID::kBytesSize + sizeof(PaymentNodeID))
            + sizeof(byte);
    if (mIsReceiptContains) {
        bytesCount += sizeof(KeyNumber)
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
