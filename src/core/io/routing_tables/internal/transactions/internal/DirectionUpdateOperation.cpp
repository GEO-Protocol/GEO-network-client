#include "DirectionUpdateOperation.h"


namespace io {
namespace routing_tables {


DirectionUpdateOperation::DirectionUpdateOperation(
    const AbstractRecordsHandler::RecordNumber recN,
    const TrustLineDirection direction,
    const TrustLineDirection directionBackup):
    Operation(Update),
    mRecN(recN),
    mDirection(direction),
    mDirectionBackup(directionBackup){}

DirectionUpdateOperation::DirectionUpdateOperation(
    const byte *data):
    Operation(Update){

    auto currentOffset = data;

    auto operationType = (Operation::SerializedOperationType*)currentOffset;
    mType = Operation::OperationType(*operationType);
    currentOffset += sizeof(Operation::SerializedOperationType);

    // Deserialization of the record number
    auto recordNumberOffset = (RecordNumber*)currentOffset;
    mRecN = *recordNumberOffset;
    currentOffset += sizeof(mRecN);

    // Deserialization of the direction
    auto directionOffset = (Operation::SerializedTrustLineDirectionType*)currentOffset;
    mDirection = (TrustLineDirection)*directionOffset;
    currentOffset += sizeof(Operation::SerializedTrustLineDirectionType);

    // Deserialization of the direction backup
    auto directionBackupOffset = (Operation::SerializedTrustLineDirectionType*)currentOffset;
    mDirectionBackup = (TrustLineDirection)*directionBackupOffset;
}

const pair<shared_ptr<byte>, size_t> DirectionUpdateOperation::serialize() const {
    const auto bufferSize = kSerializedSize;

    auto *buffer = (byte*)malloc(bufferSize);
    if (buffer == nullptr) {
        throw MemoryError(
            "SetOperation::serialize: bad allocation.");
    }
    shared_ptr<byte> bufferHandler(buffer, free);


    auto currentOffset = buffer;

    // Serializing operation type
    Operation::SerializedOperationType serializableOperationType = mType;
    memcpy(currentOffset, &serializableOperationType, sizeof(serializableOperationType));
    currentOffset += sizeof(serializableOperationType);

    // Serializing record number
    memcpy(currentOffset, &mRecN, sizeof(mRecN));
    currentOffset += sizeof(mRecN);

    // Serializing direction
    SerializedTrustLineDirectionType serializableDirection = mDirection;
    memcpy(currentOffset, &serializableDirection, sizeof(serializableDirection));
    currentOffset += sizeof(serializableDirection);

    // Serializing direction backup
    SerializedTrustLineDirectionType serializableDirectionBackup = mDirectionBackup;
    memcpy(currentOffset, &serializableDirectionBackup, sizeof(serializableDirectionBackup));

    return make_pair(bufferHandler, bufferSize);
}

const RollbackDirectionUpdateOperation::Shared DirectionUpdateOperation::rollbackOperation() const {
    try {
        return RollbackDirectionUpdateOperation::Shared(
            new RollbackDirectionUpdateOperation(mRecN, mDirectionBackup));

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
    } catch (std::bad_alloc &) {
        throw MemoryError(
            "SetOperation::rollbackOperation: bad allocation.");
#pragma clang diagnostic pop
    }
}

const AbstractRecordsHandler::RecordNumber DirectionUpdateOperation::recordNumber() const {
    return mRecN;
}

const TrustLineDirection DirectionUpdateOperation::direction() const {
    return mDirection;
}

const TrustLineDirection DirectionUpdateOperation::directionBackup() const {
    return mDirectionBackup;
}


}
}


