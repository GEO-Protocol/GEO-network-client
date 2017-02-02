#include "RoutingTableOutgoingMessage.h"

RoutingTableOutgoingMessage::RoutingTableOutgoingMessage(
    NodeUUID &sender,
    TransactionUUID &transactionUUID,
    NodeUUID &contractor) :

    Message(
        sender,
        transactionUUID
    ),
    mContractor(contractor){

    try {
        mRecords = unique_ptr<map<NodeUUID, TrustLineDirection>>(new map<NodeUUID, TrustLineDirection>);

    } catch (std::bad_alloc&) {
        throw MemoryError("RoutingTableOutgoingMessage::RoutingTableOutgoingMessage: "
                              "Can not allocate memory for routing table records container.");
    }
}

RoutingTableOutgoingMessage::~RoutingTableOutgoingMessage() {}

pair<ConstBytesShared, size_t> RoutingTableOutgoingMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
        NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        NodeUUID::kBytesSize +
        sizeof(uint32_t) +
        (mRecords->size() * (NodeUUID::kBytesSize + sizeof(uint8_t)));

    byte *data = (byte *) calloc(dataSize, sizeof(byte));
    size_t dataBufferOffset = 0;
    //---------------------------------------------------
    uint16_t type = typeID();
    memcpy(
        data,
        &type,
        sizeof(uint16_t)
    );
    dataBufferOffset += sizeof(uint16_t);
    //---------------------------------------------------
    memcpy(
        data + dataBufferOffset,
        mSenderUUID.data,
        NodeUUID::kBytesSize
    );
    dataBufferOffset += NodeUUID::kBytesSize;
    //---------------------------------------------------
    memcpy(
        data + dataBufferOffset,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    dataBufferOffset += TransactionUUID::kBytesSize;
    //---------------------------------------------------
    memcpy(
        data + dataBufferOffset,
        mContractor.data,
        NodeUUID::kBytesSize
    );
    dataBufferOffset += NodeUUID::kBytesSize;
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
      ConstBytesShared(
          data,
          free
      ),
      dataSize
    );

}

void RoutingTableOutgoingMessage::deserialize(
    byte *buffer) {

    throw NotImplementedError("RoutingTableOutgoingMessage::deserialize: "
                                  "Method not implemented.");
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