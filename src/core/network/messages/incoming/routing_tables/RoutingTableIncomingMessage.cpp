#include "RoutingTableIncomingMessage.h"

RoutingTableIncomingMessage::RoutingTableIncomingMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}


pair<BytesShared, size_t> RoutingTableIncomingMessage::serializeToBytes() {

    throw NotImplementedError("RoutingTableIncomingMessage::serializeToBytes: "
                                  "Method not implemented.");
}

void RoutingTableIncomingMessage::deserializeFromBytes(
    BytesShared buffer) {

    RoutingTablesMessage::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = RoutingTablesMessage::kOffsetToInheritedBytes();
    //---------------------------------------------------
    RecordsCount *nodesCount = new (buffer.get() + bytesBufferOffset) RecordsCount;
    bytesBufferOffset += sizeof(RecordsCount);
    //---------------------------------------------------
    for (size_t nodesIterator = 0; nodesIterator < *nodesCount; ++nodesIterator) {
        NodeUUID node;
        memcpy(
          node.data,
          buffer.get() + bytesBufferOffset,
          NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        //---------------------------------------------------
        RecordsCount *recordsCount = new (buffer.get() + bytesBufferOffset) RecordsCount;
        bytesBufferOffset += sizeof(RecordsCount);
        //---------------------------------------------------
        vector<pair<NodeUUID, TrustLineDirection>> records;
        records.clear();
        for (size_t recordsIterator = 0; recordsIterator < *recordsCount; ++recordsIterator) {
            NodeUUID neighbor;
            memcpy(
              neighbor.data,
              buffer.get() + bytesBufferOffset,
              NodeUUID::kBytesSize
            );
            bytesBufferOffset += NodeUUID::kBytesSize;
            //---------------------------------------------------
            SerializedTrustLineDirection *direct = new (buffer.get() + bytesBufferOffset) SerializedTrustLineDirection;
            bytesBufferOffset += sizeof(SerializedTrustLineDirection);
            TrustLineDirection direction = (TrustLineDirection) *direct;
            //---------------------------------------------------
            records.push_back(
                make_pair(
                    neighbor,
                    direction
                )
            );
        }
        //---------------------------------------------------
        mRecords.insert(
            make_pair(
                node,
                records
            )
        );
    }
    //---------------------------------------------------
}
