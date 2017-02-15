#include "GetTopologyAndBalancesMessageFirstLevelNode.h"


//void GetTopologyAndBalancesMessageFirstLevelNode::deserializeFromBytes(BytesShared buffer) {
//    Message::deserializeFromBytes(buffer);
//}

GetTopologyAndBalancesMessageFirstLevelNode::GetTopologyAndBalancesMessageFirstLevelNode(const TrustLineAmount maxFlow,
                                                                                         const byte max_depth,
                                                                                         vector<NodeUUID> &path) {
    mMaxFlow = maxFlow;
    mMax_depth = max_depth;
    mPath = path;
}

GetTopologyAndBalancesMessageFirstLevelNode::GetTopologyAndBalancesMessageFirstLevelNode(BytesShared buffer) {

}

pair<BytesShared, size_t> GetTopologyAndBalancesMessageFirstLevelNode::serializeToBytes() {
    auto parentBytesAndCount = Message::serializeToBytes();


    vector<byte> MaxFlowBuffer = trustLineAmountToBytes(mMaxFlow);
    uint8_t path_size = (uint8_t) mPath.size();
    size_t bytesCount = parentBytesAndCount.second +
                                MaxFlowBuffer.size() +
                                sizeof(mMax_depth) +
                                sizeof(path_size) +
                                path_size * NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

    // for parent node
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    // for max flow
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            MaxFlowBuffer.data(),
            MaxFlowBuffer.size()
    );
    dataBytesOffset += MaxFlowBuffer.size();
    //----------------------------------------------------
    // for max depth
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mMax_depth,
            sizeof(mMax_depth)
    );
    dataBytesOffset += sizeof(mMax_depth);
    //----------------------------------------------------
    // For path
    // Write vector size first
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &path_size,
            sizeof(path_size)
    );
    dataBytesOffset += sizeof(path_size);
    for(auto const& value: mPath) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &value,
            NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}


void GetTopologyAndBalancesMessageFirstLevelNode::deserializeFromBytes(
        BytesShared buffer) {

    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}


const Message::MessageType GetTopologyAndBalancesMessageFirstLevelNode::typeID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}

const TransactionUUID &GetTopologyAndBalancesMessageFirstLevelNode::transactionUUID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}

const TrustLineUUID &GetTopologyAndBalancesMessageFirstLevelNode::trustLineUUID() const {
    throw NotImplementedError("OpenTrustLineMessage::deserializeFromBytes: "
                                      "Method not implemented.");
}