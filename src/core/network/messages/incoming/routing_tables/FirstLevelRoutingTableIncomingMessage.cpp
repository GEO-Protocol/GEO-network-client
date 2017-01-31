#include "FirstLevelRoutingTableIncomingMessage.h"

FirstLevelRoutingTableIncomingMessage::FirstLevelRoutingTableIncomingMessage(
    byte *buffer) {

    deserialize(buffer);
}

FirstLevelRoutingTableIncomingMessage::~FirstLevelRoutingTableIncomingMessage() {}

pair<ConstBytesShared, size_t> FirstLevelRoutingTableIncomingMessage::serialize() {

}

void FirstLevelRoutingTableIncomingMessage::deserialize(
    byte *buffer) {

    //---------------------------------------------------
    memcpy(
        mSenderUUID.data,
        buffer,
        NodeUUID::kBytesSize
    );
    //---------------------------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer + NodeUUID::kBytesSize,
        TransactionUUID::kBytesSize
    );
    //---------------------------------------------------
    memcpy(
        mContractor.data,
        buffer + NodeUUID::kBytesSize + TransactionUUID::kBytesSize,
        NodeUUID::kBytesSize
    );
    //---------------------------------------------------
    uint32_t *recordsCount = new (buffer + NodeUUID::kBytesSize + TransactionUUID::kBytesSize + NodeUUID::kBytesSize) uint32_t;
    //---------------------------------------------------
    size_t bytesBufferOffset = NodeUUID::kBytesSize +
        TransactionUUID::kBytesSize +
        NodeUUID::kBytesSize +
        sizeof(uint32_t);

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

        mRecords->insert(
            make_pair(
                neighbor,
                trustLineDirection
            )
        );
    }

}

const Message::MessageTypeID FirstLevelRoutingTableIncomingMessage::typeID() const {

    return Message::MessageTypeID::FirstLevelRoutingTableIncomingMessageType;
}


