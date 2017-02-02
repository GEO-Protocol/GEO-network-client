#include "FirstLevelRoutingTableOutgoingMessage.h"

FirstLevelRoutingTableOutgoingMessage::FirstLevelRoutingTableOutgoingMessage(
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
        throw MemoryError("FirstLevelRoutingTableOutgoingMessage::FirstLevelRoutingTableOutgoingMessage: "
                              "Can not allocate memory for routing table records container.");
    }
}

FirstLevelRoutingTableOutgoingMessage::~FirstLevelRoutingTableOutgoingMessage() {}

pair<ConstBytesShared, size_t> FirstLevelRoutingTableOutgoingMessage::serialize() {

    size_t dataSize = sizeof(uint16_t) +
        NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        NodeUUID::kBytesSize +
        (NodeUUID::kBytesSize * (mRecords->size() + sizeof(uint8_t)));
    byte *data = (byte *) calloc(dataSize, sizeof(byte));
    //---------------------------------------------------
    uint16_t type = typeID();
    memcpy(
        data,
        &type,
        sizeof(uint16_t)
    );
    //---------------------------------------------------
    memcpy(
        data + sizeof(uint16_t),
        mSenderUUID.data,
        NodeUUID::kBytesSize
    );
    //---------------------------------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize,
        mTransactionUUID.data,
        TransactionUUID::kBytesSize
    );
    //---------------------------------------------------
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        mContractor.data,
        NodeUUID::kBytesSize
    );
    //---------------------------------------------------
    uint32_t recordsCount = mRecords->size();
    memcpy(
        data + sizeof(uint16_t) + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + NodeUUID::kBytesSize,
        &recordsCount,
        sizeof(uint32_t)
    );
    //---------------------------------------------------
    size_t dataBufferOffset = sizeof(uint16_t) +
        NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        NodeUUID::kBytesSize +
        sizeof(uint32_t);

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

void FirstLevelRoutingTableOutgoingMessage::deserialize(
    byte *buffer) {

    throw NotImplementedError("FirstLevelRoutingTableOutgoingMessage::deserialize: "
                                  "Method not implemented.");
}

void FirstLevelRoutingTableOutgoingMessage::pushBack(
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
        throw MemoryError("FirstLevelRoutingTableOutgoingMessage::pushBack: "
                              "Can not reallocate memory when insert new element in routing table container.");
    }
}

const Message::MessageTypeID FirstLevelRoutingTableOutgoingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableOutgoingMessageType;
}