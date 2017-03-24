#include "ResultRoutingTablesMessage.h"

ResultRoutingTablesMessage::ResultRoutingTablesMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    vector<pair<NodeUUID, TrustLineDirection>> rt1):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT1(rt1) {}


ResultRoutingTablesMessage::ResultRoutingTablesMessage(
        BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultRoutingTablesMessage::typeID() const {
    return Message::MessageTypeID::ResultRoutingTablesMessageType;
}

// TODO: add serialize direction
pair<BytesShared, size_t> ResultRoutingTablesMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
            sizeof(uint32_t) + mRT1.size() * (NodeUUID::kBytesSize);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    uint32_t trustLinesOutCount = (uint32_t)mRT1.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &trustLinesOutCount,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mRT1) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            it.first.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        // TODO: serialize TrustLineDirection
    }
    //----------------------------------------------------
    return make_pair(
            dataBytesShared,
            bytesCount);
}

// TODO: add deserialize direction
void ResultRoutingTablesMessage::deserializeFromBytes(
        BytesShared buffer){

    TransactionMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    uint32_t *rt1Count = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mRT1.clear();
    mRT1.reserve(*rt1Count);
    for (int idx = 0; idx < *rt1Count; idx++) {
        NodeUUID nodeUUID;
        memcpy(
            nodeUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        //TODO: deserialize TrustLineDirection
        //---------------------------------------------------
        mRT1.push_back(make_pair(nodeUUID, TrustLineDirection::Both));
    }
    //-----------------------------------------------------
}
