#include "SetOperation.h"


namespace io {
namespace routing_tables {


SetOperation::SetOperation(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirection direction,
    const AbstractRecordsHandler::RecordNumber recN):
    Operation(Set),
    mU1(u1),
    mU2(u2),
    mDirection(direction),
    mRecN(recN){}

/*
 * data doesn't contains transaction transactionType.
 */
SetOperation::SetOperation(
    const AbstractRecordsHandler::byte *data):
    Operation(Set){

    auto currentOffset = data;

    // Set operation transactionType
    // This constructor is shared also for RemoveOperation.
    // So the transactionType should be loaded from file,
    // instead of direct setting.
    //
    // (see RemoveOperation.h for details)
    auto operationType = (Operation::SerializedOperationType*)currentOffset;
    mType = Operation::OperationType(*operationType);
    currentOffset += sizeof(Operation::SerializedOperationType);


    // Deserialization of the first node transactionUUID
    memcpy(mU1.data, currentOffset, NodeUUID::kHexSize);
    currentOffset += NodeUUID::kHexSize;

    // Deserialization of the second node transactionUUID
    memcpy(mU2.data, currentOffset, NodeUUID::kHexSize);
    currentOffset += NodeUUID::kHexSize;

    // Deserialization of the direction
    auto directionOffset = (Operation::SerializedTrustLineDirectionType*)currentOffset;
    mDirection = (TrustLineDirection)*directionOffset;
    currentOffset += sizeof(Operation::SerializedTrustLineDirectionType);

    // Deserialization of the record number
    auto recordNumberOffset = (RecordNumber*)currentOffset;
    mRecN = *recordNumberOffset;
}

const pair<shared_ptr<byte>, size_t> SetOperation::serialize() const {
    const auto bufferSize = kSerializedSize;

    auto *buffer = (byte*)malloc(bufferSize);
    if (buffer == nullptr) {
        throw MemoryError(
            "SetOperation::serialize: bad allocation.");
    }
    shared_ptr<byte> bufferHandler(buffer, free);


    auto currentOffset = buffer;

    // Serializing operation transactionType
    Operation::SerializedOperationType serializableOperationType = mType;
    memcpy(currentOffset, &serializableOperationType, sizeof(serializableOperationType));
    currentOffset += sizeof(serializableOperationType);

    // Serializing first node transactionUUID
    memcpy(currentOffset, mU1.data, NodeUUID::kHexSize);
    currentOffset += NodeUUID::kHexSize;

    // Serializing second node transactionUUID
    memcpy(currentOffset, mU2.data, NodeUUID::kHexSize);
    currentOffset += NodeUUID::kHexSize;

    // Serializing direction
    SerializedTrustLineDirectionType serializableDirection = mDirection;
    memcpy(currentOffset, &serializableDirection, sizeof(serializableDirection));
    currentOffset += sizeof(serializableDirection);

    // Serializing record number
    memcpy(currentOffset, &mRecN, sizeof(mRecN));

    return make_pair(bufferHandler, bufferSize);
}

const RollbackSetOperation::Shared SetOperation::rollbackOperation() const {
    try {
        return RollbackSetOperation::Shared(
            new RollbackSetOperation(mU1, mU2));

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
    } catch (std::bad_alloc &) {
        throw MemoryError(
            "SetOperation::rollbackOperation: bad allocation.");
#pragma clang diagnostic pop
    }
}

const NodeUUID &SetOperation::u1() const {
    return mU1;
}

const NodeUUID &SetOperation::u2() const {
    return mU2;
}

const TrustLineDirection SetOperation::direction() const {
    return mDirection;
}

const AbstractRecordsHandler::RecordNumber SetOperation::recordNumber() const {
    return mRecN;
}


}
}

