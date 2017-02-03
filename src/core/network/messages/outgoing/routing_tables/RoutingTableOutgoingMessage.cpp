#include "RoutingTableOutgoingMessage.h"

RoutingTableOutgoingMessage::RoutingTableOutgoingMessage(
    NodeUUID &senderUUID,
    NodeUUID &contractorUUID,
    TrustLineUUID &trustLineUUID) :

    RoutingTablesMessage(
        senderUUID,
        contractorUUID,
        trustLineUUID
    ) {

    try {
        mRecords = unique_ptr<map<NodeUUID, TrustLineDirection>>(new map<NodeUUID, TrustLineDirection>);

    } catch (std::bad_alloc&) {
        throw MemoryError("RoutingTableOutgoingMessage::RoutingTableOutgoingMessage: "
                              "Can not allocate memory for routing table records container.");
    }
}

void RoutingTableOutgoingMessage::pushBack(
    NodeUUID &neighbor,
    TrustLineDirection direction) {

    try {
        mRecords->insert(
            make_pair(
                neighbor,
                direction
            )
        );

    } catch (std::bad_alloc&) {
        throw MemoryError("RoutingTableOutgoingMessage::pushBack: "
                              "Can not reallocate memory when insert new element in routing table container.");
    }
}

pair<ConstBytesShared, size_t> RoutingTableOutgoingMessage::serialize() {

    auto parentBytesAndCount = serializeParentToBytes();

    size_t dataSize = parentBytesAndCount.second + (mRecords->size() * (NodeUUID::kBytesSize + sizeof(uint8_t)));
    byte *data = (byte *) calloc(dataSize, sizeof(byte));

    memcpy(
        data,
        const_cast<byte *> (parentBytesAndCount.first.get()),
        parentBytesAndCount.second
    );
    size_t dataBufferOffset = parentBytesAndCount.second;

    //---------------------------------------------------
    uint32_t recordsCount = mRecords->size();
    memcpy(
        data + dataBufferOffset,
        &recordsCount,
        sizeof(uint32_t)
    );
    dataBufferOffset += sizeof(uint32_t);
    //---------------------------------------------------
    for (auto const &neighborAndDirect : *mRecords) {
        memcpy(
          data + dataBufferOffset,
          neighborAndDirect.first.data,
          NodeUUID::kBytesSize
        );
        dataBufferOffset += NodeUUID::kBytesSize;

        uint8_t direction = neighborAndDirect.second;
        memcpy(
          data + dataBufferOffset,
          &direction,
          sizeof(uint8_t)
        );
        dataBufferOffset += sizeof(uint8_t);
    }
    //---------------------------------------------------
    return make_pair(
      ConstBytesShared(data, free),
      dataSize
    );

}

void RoutingTableOutgoingMessage::deserialize(
    byte *buffer) {

    throw NotImplementedError("RoutingTableOutgoingMessage::deserialize: "
                                  "Method not implemented.");
}