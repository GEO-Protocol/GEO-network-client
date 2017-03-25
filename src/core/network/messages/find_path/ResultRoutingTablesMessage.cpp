#include "ResultRoutingTablesMessage.h"

ResultRoutingTablesMessage::ResultRoutingTablesMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID &transactionUUID,
    vector<pair<const NodeUUID, const TrustLineDirection>> rt1,
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> rt2,
    vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> rt3):

    TransactionMessage(
        senderUUID,
        transactionUUID),

    mRT1(rt1),
    mRT2(rt2),
    mRT3(rt3){}

ResultRoutingTablesMessage::ResultRoutingTablesMessage(
        BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType ResultRoutingTablesMessage::typeID() const {
    return Message::MessageTypeID::ResultRoutingTablesMessageType;
}

vector<pair<const NodeUUID, const TrustLineDirection>> ResultRoutingTablesMessage::rt1() {

    return mRT1;
};

vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> ResultRoutingTablesMessage::rt2() {

    return mRT2;
};

vector<tuple<NodeUUID, NodeUUID, TrustLineDirection>> ResultRoutingTablesMessage::rt3() {

    return mRT3;
};

pair<BytesShared, size_t> ResultRoutingTablesMessage::serializeToBytes() {

    auto parentBytesAndCount = TransactionMessage::serializeToBytes();
    size_t bytesCount = parentBytesAndCount.second +
            sizeof(uint32_t) + mRT1.size() * (NodeUUID::kBytesSize + sizeof(SerializedTrustLineDirection)) +
            sizeof(uint32_t) + mRT2.size() * (2 * NodeUUID::kBytesSize + sizeof(SerializedTrustLineDirection)) +
            sizeof(uint32_t) + mRT3.size() * (2 * NodeUUID::kBytesSize + sizeof(SerializedTrustLineDirection));
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    uint32_t rt1Size = (uint32_t)mRT1.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &rt1Size,
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
        SerializedTrustLineDirection serializableDirection = it.second;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &serializableDirection,
            sizeof(SerializedTrustLineDirection));
        dataBytesOffset += sizeof(SerializedTrustLineDirection);
    }
    //----------------------------------------------------
    uint32_t rt2Size = (uint32_t)mRT2.size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &rt2Size,
        sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mRT2) {
        NodeUUID source;
        NodeUUID target;
        TrustLineDirection direction;
        std::tie(source, target, direction) = it;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            source.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            target.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        SerializedTrustLineDirection serializableDirection = direction;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &serializableDirection,
            sizeof(SerializedTrustLineDirection));
        dataBytesOffset += sizeof(SerializedTrustLineDirection);
    }
    //----------------------------------------------------
    uint32_t rt3Size = (uint32_t)mRT3.size();
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &rt3Size,
            sizeof(uint32_t));
    dataBytesOffset += sizeof(uint32_t);
    //----------------------------------------------------
    for (auto const &it : mRT3) {
        NodeUUID source;
        NodeUUID target;
        TrustLineDirection direction;
        std::tie(source, target, direction) = it;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            source.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            target.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;
        //------------------------------------------------
        SerializedTrustLineDirection serializableDirection = direction;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &serializableDirection,
            sizeof(SerializedTrustLineDirection));
        dataBytesOffset += sizeof(SerializedTrustLineDirection);
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

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
        auto directionOffset = (SerializedTrustLineDirection*)(buffer.get() + bytesBufferOffset);
        TrustLineDirection direction = (TrustLineDirection)*directionOffset;
        //---------------------------------------------------
        mRT1.push_back(make_pair(nodeUUID, direction));
    }
    //-----------------------------------------------------
    uint32_t *rt2Count = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mRT2.clear();
    mRT1.reserve(*rt2Count);
    for (int idx = 0; idx < *rt2Count; idx++) {
        NodeUUID source;
        memcpy(
            source.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        NodeUUID target;
        memcpy(
            target.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        auto directionOffset = (SerializedTrustLineDirection*)(buffer.get() + bytesBufferOffset);
        TrustLineDirection direction = (TrustLineDirection)*directionOffset;
        //---------------------------------------------------
        mRT2.push_back(make_tuple(source, target, direction));
    }
    //-----------------------------------------------------
    uint32_t *rt3Count = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //-----------------------------------------------------
    mRT2.clear();
    mRT1.reserve(*rt3Count);
    for (int idx = 0; idx < *rt3Count; idx++) {
        NodeUUID source;
        memcpy(
            source.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        NodeUUID target;
        memcpy(
            target.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        auto directionOffset = (SerializedTrustLineDirection*)(buffer.get() + bytesBufferOffset);
        TrustLineDirection direction = (TrustLineDirection)*directionOffset;
        //---------------------------------------------------
        mRT3.push_back(make_tuple(source, target, direction));
    }
    //-----------------------------------------------------
}
