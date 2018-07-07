#include "ResultMaxFlowCalculationMessage.h"

ResultMaxFlowCalculationMessage::ResultMaxFlowCalculationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &outgoingFlows,
    vector<pair<NodeUUID, ConstSharedTrustLineAmount>> &incomingFlows) :

    MaxFlowCalculationConfirmationMessage(
        equivalent,
        senderUUID,
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
        NodeUUID nodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mOutgoingFlows.emplace_back(
            nodeUUID,
            make_shared<const TrustLineAmount>(
                trustLineAmount));
    }
    //----------------------------------------------------
    auto *trustLinesInCount = new (buffer.get() + bytesBufferOffset) SerializedRecordsCount;
    bytesBufferOffset += sizeof(SerializedRecordsCount);
    //-----------------------------------------------------
    mIncomingFlows.reserve(*trustLinesInCount);
    for (SerializedRecordNumber idx = 0; idx < *trustLinesInCount; idx++) {
        NodeUUID nodeUUID(buffer.get() + bytesBufferOffset);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        vector<byte> bufferTrustLineAmount(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
        bytesBufferOffset += kTrustLineAmountBytesCount;
        //---------------------------------------------------
        TrustLineAmount trustLineAmount = bytesToTrustLineAmount(bufferTrustLineAmount);
        mIncomingFlows.emplace_back(
            nodeUUID,
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
    throw(bad_alloc)
{
    auto parentBytesAndCount = MaxFlowCalculationConfirmationMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second
                        + sizeof(SerializedRecordsCount) + mOutgoingFlows.size()
                                                           * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount)
                        + sizeof(SerializedRecordsCount) + mIncomingFlows.size()
                                                           * (NodeUUID::kBytesSize + kTrustLineAmountBytesCount);
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
    for (auto const &it : mOutgoingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        vector<byte> buffer = trustLineAmountToBytes(*it.second.get());
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
    for (auto const &it : mIncomingFlows) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        vector<byte> buffer = trustLineAmountToBytes(*it.second.get());
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

const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> ResultMaxFlowCalculationMessage::outgoingFlows() const
{
    return mOutgoingFlows;
}

const vector<pair<NodeUUID, ConstSharedTrustLineAmount>> ResultMaxFlowCalculationMessage::incomingFlows() const
{
    return mIncomingFlows;
}
