#include "FinalPathCycleConfigurationMessage.h"

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<PaymentNodeID, BaseAddress::Shared> &paymentParticipants) :

    RequestCycleMessage(
        equivalent,
        senderUUID,
        senderAddresses,
        transactionUUID,
        amount),
    mPaymentParticipants(paymentParticipants),
    mIsReceiptContains(false)
{}

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<PaymentNodeID, BaseAddress::Shared> &paymentParticipants,
    const KeyNumber publicKeyNumber,
    const lamport::Signature::Shared signature) :

    RequestCycleMessage(
        equivalent,
        senderUUID,
        senderAddresses,
        transactionUUID,
        amount),
    mPaymentParticipants(paymentParticipants),
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

    auto *paymentParticipantsCount = new (bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    for (SerializedRecordNumber idx = 0; idx < *paymentParticipantsCount; idx++) {
        auto *paymentNodeID = new (bytesBufferOffset) PaymentNodeID;
        bytesBufferOffset += sizeof(PaymentNodeID);
        //---------------------------------------------------
        auto address = deserializeAddress(bytesBufferOffset);
        bytesBufferOffset += address->serializedSize();
        //---------------------------------------------------
        mPaymentParticipants.insert(
            make_pair(
                *paymentNodeID,
                address));
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

const map<PaymentNodeID, BaseAddress::Shared>& FinalPathCycleConfigurationMessage::paymentParticipants() const
{
    return mPaymentParticipants;
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
            + sizeof(byte);
    for (const auto &participant : mPaymentParticipants) {
        bytesCount += sizeof(PaymentNodeID) + participant.second->serializedSize();
    }
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
    auto paymentNodesIdsCount = (SerializedRecordsCount)mPaymentParticipants.size();
    memcpy(
        bytesBufferOffset,
        &paymentNodesIdsCount,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //----------------------------------------------------
    for (auto const &paymentNodeIdAndAddress : mPaymentParticipants) {
        memcpy(
            bytesBufferOffset,
            &paymentNodeIdAndAddress.first,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);

        auto serializedAddress = paymentNodeIdAndAddress.second->serializeToBytes();
        memcpy(
            bytesBufferOffset,
            serializedAddress.get(),
            paymentNodeIdAndAddress.second->serializedSize());
        bytesBufferOffset += paymentNodeIdAndAddress.second->serializedSize();
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
