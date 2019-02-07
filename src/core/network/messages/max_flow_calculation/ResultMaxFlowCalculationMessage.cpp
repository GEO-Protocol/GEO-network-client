#include "ResultMaxFlowCalculationMessage.h"

ResultMaxFlowCalculationMessage::ResultMaxFlowCalculationMessage(
    const SerializedEquivalent equivalent,
    vector<BaseAddress::Shared> senderAddresses,
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &outgoingFlows,
    vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> &incomingFlows) :

    MaxFlowCalculationConfirmationMessage(
        equivalent,
        senderAddresses,
        0),
    mOutgoingFlows(outgoingFlows),
    mIncomingFlows(incomingFlows)
{}

ResultMaxFlowCalculationMessage::ResultMaxFlowCalculationMessage(
    BytesShared buffer):

    MaxFlowCalculationConfirmationMessage(buffer)
{
    size_t bytesBufferOffset = MaxFlowCalculationConfirmationMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    auto *trustLinesOutCount = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    mOutgoingFlows.reserve(*trustLinesOutCount);
    for (SerializedRecordNumber idx = 0; idx < *trustLinesOutCount; idx++) {
        auto address = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += address->serializedSize();
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mOutgoingFlows.emplace_back(
            address,
            make_shared<const TrustLineAmount>(
                trustLineAmount));
    }
    //----------------------------------------------------
    auto *trustLinesInCount = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    mIncomingFlows.reserve(*trustLinesInCount);
    for (SerializedRecordNumber idx = 0; idx < *trustLinesInCount; idx++) {
        auto address = deserializeAddress(
            buffer.get() + bytesBufferOffset);
        bytesBufferOffset += address->serializedSize();
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mIncomingFlows.emplace_back(
            address,
            make_shared<const TrustLineAmount>(
                trustLineAmount));
    }
}

const Message::MessageType ResultMaxFlowCalculationMessage::typeID() const
{
    return Message::MessageType::MaxFlow_ResultMaxFlowCalculation;
}

const bool ResultMaxFlowCalculationMessage::isAddToConfirmationNotStronglyRequiredMessagesHandler() const
{
    return true;
}

pair<BytesShared, size_t> ResultMaxFlowCalculationMessage::serializeToBytes() const
{
    auto parentBytesAndCount = MaxFlowCalculationConfirmationMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount)
                        + sizeof(SerializedRecordsCount);
    for (const auto &outgoingFlow : mOutgoingFlows) {
        bytesCount += outgoingFlow.first->serializedSize() + kTrustLineAmountBytesCount;
    }
    for (const auto &incomingFlow : mIncomingFlows) {
        bytesCount += incomingFlow.first->serializedSize() + kTrustLineAmountBytesCount;
    }
    BytesShared dataBytesShared = tryCalloc(bytesCount);

    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------

    auto trustLinesOutCount = (SerializedRecordsCount)mOutgoingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesOutCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    //----------------------------------------------------
    for (auto const &outgoingFlow : mOutgoingFlows) {
        auto serializedData = outgoingFlow.first->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedData.get(),
            outgoingFlow.first->serializedSize());
        dataBytesOffset += outgoingFlow.first->serializedSize();
        //------------------------------------------------
        vector<byte> buffer = trustLineAmountToBytes(*outgoingFlow.second.get());
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
        dataBytesOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    auto trustLinesInCount = (SerializedRecordsCount)mIncomingFlows.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesInCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);
    //----------------------------------------------------
    for (auto const &incomingFlow : mIncomingFlows) {
        auto serializedData = incomingFlow.first->serializeToBytes();
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            serializedData.get(),
            incomingFlow.first->serializedSize());
        dataBytesOffset += incomingFlow.first->serializedSize();
        //------------------------------------------------
        vector<byte> buffer = trustLineAmountToBytes(*incomingFlow.second.get());
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            buffer.data(),
            buffer.size());
        dataBytesOffset += kTrustLineAmountBytesCount;
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> ResultMaxFlowCalculationMessage::outgoingFlows() const
{
    return mOutgoingFlows;
}

const vector<pair<BaseAddress::Shared, ConstSharedTrustLineAmount>> ResultMaxFlowCalculationMessage::incomingFlows() const
{
    return mIncomingFlows;
}
