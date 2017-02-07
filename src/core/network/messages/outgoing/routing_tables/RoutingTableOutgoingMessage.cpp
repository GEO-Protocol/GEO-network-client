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

pair<BytesShared, size_t> RoutingTableOutgoingMessage::serializeToBytes() {

    auto parentBytesAndCount = RoutingTablesMessage::serializeToBytes();
    size_t bytesCount = (parentBytesAndCount.second + sizeof(uint32_t)) +
        mRecords->size() * (NodeUUID::kBytesSize + sizeof(SerializedTrustLineDirection));
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //---------------------------------------------------
    uint32_t recordsCount = (uint32_t) mRecords->size();
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &recordsCount,
        sizeof(uint32_t)
    );
    dataBytesOffset += sizeof(uint32_t);
    //---------------------------------------------------
    for (auto const &neighborAndDirect : *mRecords) {
        memcpy(
          dataBytesShared.get() + dataBytesOffset,
          neighborAndDirect.first.data,
          NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;

        SerializedTrustLineDirection direction = neighborAndDirect.second;
        memcpy(
          dataBytesShared.get() + dataBytesOffset,
          &direction,
          sizeof(SerializedTrustLineDirection)
        );
        dataBytesOffset += sizeof(SerializedTrustLineDirection);
    }
    //---------------------------------------------------
    return make_pair(
      dataBytesShared,
      bytesCount
    );

}

void RoutingTableOutgoingMessage::deserializeFromBytes(
    BytesShared buffer) {

    throw NotImplementedError("RoutingTableOutgoingMessage::deserializeFromBytes: "
                                  "Method not implemented.");
}