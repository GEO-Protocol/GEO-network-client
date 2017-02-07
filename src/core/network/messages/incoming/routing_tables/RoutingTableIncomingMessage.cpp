#include "RoutingTableIncomingMessage.h"

RoutingTableIncomingMessage::RoutingTableIncomingMessage(
    byte *buffer) {

    try {
        mRecords = unique_ptr<map<NodeUUID, TrustLineDirection>>(new map<NodeUUID, TrustLineDirection>);

    } catch (std::bad_alloc&) {
        throw MemoryError("RoutingTableOutgoingMessage::RoutingTableOutgoingMessage: "
                              "Can not allocate memory for routing table records container.");
    }
    deserialize(buffer);
}


pair<ConstBytesShared, size_t> RoutingTableIncomingMessage::serialize() {

    throw NotImplementedError("RoutingTableIncomingMessage::serialize: "
                                  "Method not implemented.");
}

void RoutingTableIncomingMessage::deserialize(
    byte *buffer) {

    deserializeParentFromBytes(buffer);
    size_t bytesBufferOffset = kOffsetToInheritBytes();
    //---------------------------------------------------
    uint32_t *recordsCount = new (buffer + bytesBufferOffset) uint32_t;
    bytesBufferOffset += sizeof(uint32_t);
    //---------------------------------------------------
    for (size_t counter = 0; counter < *recordsCount; ++counter) {
        NodeUUID neighbor;
        memcpy(
            neighbor.data,
            buffer + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;

        uint8_t *direct = new (buffer + bytesBufferOffset) uint8_t;
        bytesBufferOffset += sizeof(uint8_t);

        TrustLineDirection trustLineDirection = (TrustLineDirection) *direct;

        try {
            mRecords->insert(
                make_pair(
                    neighbor,
                    trustLineDirection
                )
            );

        } catch (std::bad_alloc&) {
            throw MemoryError("RoutingTableIncomingMessage::deserialize: "
                                  "Can not reallocate memory when insert new element in routing table container.");
        }
    }
}