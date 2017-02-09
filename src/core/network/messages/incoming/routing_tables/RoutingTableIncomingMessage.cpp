#include "RoutingTableIncomingMessage.h"

RoutingTableIncomingMessage::RoutingTableIncomingMessage(
    BytesShared buffer) {

    try {
        mRecords = unique_ptr<map<NodeUUID, TrustLineDirection>>(new map<NodeUUID, TrustLineDirection>);

    } catch (std::bad_alloc&) {
        throw MemoryError("RoutingTableOutgoingMessage::RoutingTableOutgoingMessage: "
                              "Can not allocate memory for routing table records container.");
    }
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
    uint32_t *recordsCount = new (buffer.get() + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //---------------------------------------------------
    for (size_t counter = 0; counter < *recordsCount; ++counter) {
        NodeUUID neighbor;
        memcpy(
            neighbor.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;

        SerializedTrustLineDirection *direct = new (buffer.get() + bytesBufferOffset) SerializedTrustLineDirection;
        bytesBufferOffset += sizeof(SerializedTrustLineDirection);

        TrustLineDirection trustLineDirection = (TrustLineDirection) *direct;

        try {
            mRecords->insert(
                make_pair(
                    neighbor,
                    trustLineDirection
                )
            );

        } catch (std::bad_alloc&) {
            throw MemoryError("RoutingTableIncomingMessage::deserializeFromBytes: "
                                  "Can not reallocate memory when insert new element in routing table container.");
        }
    }
}
