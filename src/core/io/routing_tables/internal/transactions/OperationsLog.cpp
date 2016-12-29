#include "OperationsLog.h"



namespace io {
namespace routing_tables {


OperationsLog::OperationsLog(
    const fs::path &path):
    AbstractFileDescriptorHandler(path / fs::path("operations.dat")) {

    open(kWriteAccessMode);
}

OperationsLog::FileHeader::FileHeader():
    version(1),
    currentRecordNumber(0) {}

const SetOperation::Shared OperationsLog::initSetOperation(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirection direction) {

    try {
        SetOperation::Shared operation(
            new SetOperation(u1, u2, direction, mCurrentRecordNumber));

        logOperation(operation);
        return operation;

    } catch (std::bad_alloc &) {
        throw MemoryError(
            "OperationsLog::initSetOperation: bad alloc.");

    } catch (IOError &) {
        throw Exception(
            "OperationsLog::initSetOperation: "
                "can't initialise new transaction.");
    }
}

const RemoveOperation::Shared OperationsLog::initRemoveOperation(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirection direction) {

    try {
        RemoveOperation::Shared operation(
            new RemoveOperation(u1, u2, direction));

        logOperation(operation);
        return operation;

    } catch (std::bad_alloc &) {
        throw MemoryError(
            "OperationsLog::initRemoveOperation: bad allocation.");

    } catch (IOError &) {
        throw Exception(
            "OperationsLog::initRemoveOperation: "
                "can't initialise new transaction.");
    }
}

const DirectionUpdateOperation::Shared OperationsLog::initDirectionUpdateOperation(
    const AbstractRecordsHandler::RecordNumber recN,
    const TrustLineDirection direction,
    const TrustLineDirection directionBackup) {

    try {
        DirectionUpdateOperation::Shared operation(
            new DirectionUpdateOperation(recN, direction, directionBackup));

        logOperation(operation);
        return operation;

    } catch (std::bad_alloc &) {
        throw MemoryError(
            "OperationsLog::initDirectionUpdateOperation: bad allocation.");

    } catch (IOError &) {
        throw Exception(
            "OperationsLog::initDirectionUpdateOperation: "
                "can't initialise new transaction.");
    }
}

void OperationsLog::open(
    const char *accessMode) {

    db::AbstractFileDescriptorHandler::open(accessMode);
    if (fileSize() == 0) {
        // Init default header.
        FileHeader header; // will be initialised to the defaults by the constructor.
        updateFileHeader(&header);
        mCurrentRecordNumber = header.currentRecordNumber;

    } else {
        // Load data from current header.
        auto header = loadFileHeader();
        if (header.version != 1) {
            throw Exception(
                "OperationsLog::open: "
                    "unexpected version occurred.");
        }
        mCurrentRecordNumber = header.currentRecordNumber;
    }
}


OperationsLog::FileHeader OperationsLog::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "OperationsLog::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}

/*!
 * Atomically updates file header.
 * Throws IOError in case when write operation failed.
 */
void OperationsLog::updateFileHeader(const FileHeader *header) const {
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(FileHeader), 1, mFileDescriptor) != 1) {
        throw IOError(
            "OperationsLog::updateFileHeader: "
                "can't write file header to the disk.");
    }
    syncLowLevelOSBuffers();
}

void OperationsLog::logOperation(
    const Operation::Shared operation) {

    auto serialisationResult = operation->serialize();
    const size_t dataBytesCount = serialisationResult.second;
    const auto data = serialisationResult.first;

    fseek(mFileDescriptor, 0, SEEK_END);
    if (fwrite(data.get(), dataBytesCount, 1, mFileDescriptor) != 1) {
        throw IOError(
            "OperationsLog::logOperation: "
                "can't write operation block.");
    }
    syncLowLevelOSBuffers();
}

/*
 * operations are handled in reverse order
 */
shared_ptr<vector<Operation::Shared>> OperationsLog::uncompletedOperations(){
    const size_t operationsDataSize = fileSize() - sizeof(FileHeader);
    if (operationsDataSize == 0){
        return shared_ptr<vector<Operation::Shared>>(
            new vector<Operation::Shared>());
    }

    auto operationsDataBuffer = (byte*)malloc(operationsDataSize);
    if (operationsDataBuffer == nullptr) {
        throw MemoryError(
            "OperationsLog::uncompletedOperations: bad allocation.");
    }
    shared_ptr<byte> operationsDataBufferHandler(operationsDataBuffer, free);


    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);
    if (fread(operationsDataBuffer, operationsDataSize, 1, mFileDescriptor) != 1 &&
        fread(operationsDataBuffer, operationsDataSize, 1, mFileDescriptor) != 1) {
        throw IOError(
            "OperationsLog::uncompletedOperations: "
                "can't read operations info.");
    }

    try {
        auto operations = new vector<Operation::Shared>;
        byte *currentDataBufferOffset = (byte*)operationsDataBuffer;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
        while (currentDataBufferOffset < (operationsDataBuffer + operationsDataSize)) {

            Operation::SerializedOperationType *operationType = currentDataBufferOffset;
            switch (*operationType) {
                case Operation::Set: {
                    auto operation = Operation::Shared(new SetOperation(currentDataBufferOffset));
                    operations->insert(operations->cbegin(), operation);
                    currentDataBufferOffset += SetOperation::kSerializedSize;
                    break;
                }

                case Operation::Remove: {
                    auto operation = Operation::Shared(new RemoveOperation(currentDataBufferOffset));
                    operations->insert(operations->cbegin(), operation);
                    currentDataBufferOffset += RemoveOperation::kSerializedSize;
                    break;
                }

                case Operation::Update: {
                    auto operation = Operation::Shared(new DirectionUpdateOperation(currentDataBufferOffset));
                    operations->insert(operations->cbegin(), operation);
                    currentDataBufferOffset += DirectionUpdateOperation::kSerializedSize;
                    break;
                }

                default: {
                    throw RuntimeError(
                        "OperationsLog::uncompletedOperations: can't load operations info.");
                }
            }
        }

        // Operations must be rolled back in reverse order
#pragma clang diagnostic pop
        return shared_ptr<vector<Operation::Shared>>(operations);

    } catch (std::bad_alloc&) {
        throw MemoryError(
            "OperationsLog::uncompletedOperations: bad allocation.");
    }
}

void OperationsLog::commit() {
    truncateOperations();
}

bool OperationsLog::transactionMayBeStarted() {
    // If file doesn't contains any operations -
    // new transaction may be started.
    return fileSize() == sizeof(FileHeader);
}

void OperationsLog::truncateOperations() {
    ftruncate(fileno(mFileDescriptor), sizeof(FileHeader));
}


} // namespace routing_tables;
} // namespace io;


