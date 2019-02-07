#include "FinalPathCycleConfigurationMessage.h"

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<PaymentNodeID, Contractor::Shared> &paymentParticipants,
    const BlockNumber maximalClaimingBlockNumber) :

    RequestCycleMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        amount),
    mPaymentParticipants(paymentParticipants),
    mMaximalClaimingBlockNumber(maximalClaimingBlockNumber),
    mIsReceiptContains(false)
{}

FinalPathCycleConfigurationMessage::FinalPathCycleConfigurationMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> &senderAddresses,
    const TransactionUUID &transactionUUID,
    const TrustLineAmount &amount,
    const map<PaymentNodeID, Contractor::Shared> &paymentParticipants,
    const BlockNumber maximalClaimingBlockNumber,
    const KeyNumber publicKeyNumber,
    const lamport::Signature::Shared signature,
    const lamport::KeyHash::Shared transactionPublicKeyHash) :

    RequestCycleMessage(
        equivalent,
        senderAddresses,
        transactionUUID,
        amount),
    mPaymentParticipants(paymentParticipants),
    mMaximalClaimingBlockNumber(maximalClaimingBlockNumber),
    mIsReceiptContains(true),
    mPublicKeyNumber(publicKeyNumber),
    mSignature(signature),
    mTransactionPublicKeyHash(transactionPublicKeyHash)
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
        auto contractor = make_shared<Contractor>(bytesBufferOffset);
        bytesBufferOffset += contractor->serializedSize();
        //---------------------------------------------------
        mPaymentParticipants.insert(
            make_pair(
                *paymentNodeID,
                contractor));
    }
    //----------------------------------------------------
    memcpy(
        &mMaximalClaimingBlockNumber,
        bytesBufferOffset,
        sizeof(BlockNumber));
    bytesBufferOffset += sizeof(BlockNumber);
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
        bytesBufferOffset += lamport::Signature::signatureSize();

        mTransactionPublicKeyHash = make_shared<lamport::KeyHash>(
            bytesBufferOffset);
    }
}

const Message::MessageType FinalPathCycleConfigurationMessage::typeID() const
{
    return Message::Payments_FinalPathCycleConfiguration;
}

const map<PaymentNodeID, Contractor::Shared>& FinalPathCycleConfigurationMessage::paymentParticipants() const
{
    return mPaymentParticipants;
}

const BlockNumber FinalPathCycleConfigurationMessage::maximalClaimingBlockNumber() const
{
    return mMaximalClaimingBlockNumber;
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

const lamport::KeyHash::Shared FinalPathCycleConfigurationMessage::transactionPublicKeyHash() const
{
    return mTransactionPublicKeyHash;
}

pair<BytesShared, size_t> FinalPathCycleConfigurationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = RequestCycleMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
            + sizeof(SerializedRecordsCount)
            + sizeof(BlockNumber)
            + sizeof(byte);
    for (const auto &participant : mPaymentParticipants) {
        bytesCount += sizeof(PaymentNodeID) + participant.second->serializedSize();
    }
    if (mIsReceiptContains) {
        bytesCount += sizeof(KeyNumber)
                + lamport::Signature::signatureSize()
                + lamport::KeyHash::kBytesSize;
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
    for (auto const &paymentNodeIdAndContractor : mPaymentParticipants) {
        memcpy(
            bytesBufferOffset,
            &paymentNodeIdAndContractor.first,
            sizeof(PaymentNodeID));
        bytesBufferOffset += sizeof(PaymentNodeID);

        auto contractorSerializedData = paymentNodeIdAndContractor.second->serializeToBytes();
        memcpy(
            bytesBufferOffset,
            contractorSerializedData.get(),
            paymentNodeIdAndContractor.second->serializedSize());
        bytesBufferOffset += paymentNodeIdAndContractor.second->serializedSize();
    }
    //----------------------------------------------------
    memcpy(
        bytesBufferOffset,
        &mMaximalClaimingBlockNumber,
        sizeof(BlockNumber));
    bytesBufferOffset += sizeof(BlockNumber);
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
        bytesBufferOffset += lamport::Signature::signatureSize();

        memcpy(
            bytesBufferOffset,
            mTransactionPublicKeyHash->data(),
            lamport::KeyHash::kBytesSize);
    }
    //----------------------------------------------------
    return make_pair(
        buffer,
        bytesCount);
}
