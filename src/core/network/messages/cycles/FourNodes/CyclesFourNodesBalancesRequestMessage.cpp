#include "CyclesFourNodesBalancesRequestMessage.h"

CyclesFourNodesBalancesRequestMessage::CyclesFourNodesBalancesRequestMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const NodeUUID &creditorNeighbor,
    const NodeUUID &debtorNeighbor):

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mCreditorUUID(creditorNeighbor),
    mDebtorUUID(debtorNeighbor)
{}

CyclesFourNodesBalancesRequestMessage::CyclesFourNodesBalancesRequestMessage(
    BytesShared buffer):

    TransactionMessage(buffer)
{
    size_t bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    // DebtorUUID
    memcpy(
        mDebtorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    // CreditorUUID
    memcpy(
        mCreditorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
}

pair<BytesShared, size_t> CyclesFourNodesBalancesRequestMessage::serializeToBytes() const
    throw(bad_alloc)
{
    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + NodeUUID::kBytesSize
                        + NodeUUID::kBytesSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;

    // For mDebtor
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mDebtorUUID,
        NodeUUID::kBytesSize
    );
    dataBytesOffset += NodeUUID::kBytesSize;

    // For mCreditor
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mCreditorUUID,
        NodeUUID::kBytesSize
    );

    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

const Message::MessageType CyclesFourNodesBalancesRequestMessage::typeID() const {
    return Message::MessageType::Cycles_FourNodesBalancesRequest;
}

const NodeUUID CyclesFourNodesBalancesRequestMessage::debtor() const{
    return mDebtorUUID;
}

const NodeUUID CyclesFourNodesBalancesRequestMessage::creditor() const{
    return mCreditorUUID;
}