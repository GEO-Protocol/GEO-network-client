#include "RoutingTableIncomingMessage.h"

RoutingTableIncomingMessage::RoutingTableIncomingMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}


pair<BytesShared, size_t> RoutingTableIncomingMessage::serializeToBytes() {

    throw Exception("RoutingTableIncomingMessage::serializeToBytes: "
                                  "Method not implemented.");
}

void RoutingTableIncomingMessage::deserializeFromBytes(
    BytesShared buffer) {

    SenderMessage::deserializeFromBytes(
        buffer);
    size_t bytesBufferOffset = SenderMessage::kOffsetToInheritedBytes();
    //---------------------------------------------------
    SerializedPropagationStep *propagationStep = new (buffer.get() + bytesBufferOffset) SerializedPropagationStep;
    bytesBufferOffset += sizeof(SerializedPropagationStep);
    mPropagationStep = (RoutingTablesMessage::PropagationStep) *propagationStep;
    //---------------------------------------------------
    RecordsCount *nodesCount = new (buffer.get() + bytesBufferOffset) RecordsCount;
    bytesBufferOffset += sizeof(RecordsCount);
    //---------------------------------------------------
    for (size_t nodesIterator = 0; nodesIterator < *nodesCount; ++nodesIterator) {
        NodeUUID node;
        memcpy(
          node.data,
          buffer.get() + bytesBufferOffset,
          NodeUUID::kBytesSize);
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        RecordsCount *recordsCount = new (buffer.get() + bytesBufferOffset) RecordsCount;
        bytesBufferOffset += sizeof(RecordsCount);
        //---------------------------------------------------
        vector<NodeUUID> records;
        records.clear();
        for (size_t recordsIterator = 0; recordsIterator < *recordsCount; ++recordsIterator) {
            NodeUUID neighbor;
            memcpy(
              neighbor.data,
              buffer.get() + bytesBufferOffset,
              NodeUUID::kBytesSize);
            bytesBufferOffset += NodeUUID::kBytesSize;
            //---------------------------------------------------
            records.push_back(
                neighbor);
        }
        //---------------------------------------------------
        mRecords.insert(
            make_pair(
                node,
                records));
    }
    //---------------------------------------------------
}
