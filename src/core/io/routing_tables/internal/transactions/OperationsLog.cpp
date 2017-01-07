#include "OperationsLog.h"



namespace io {
namespace routing_tables {


OperationsLog::OperationsLog(
    const fs::path &path):
    AbstractFileDescriptorHandler(path / fs::path("operations.dat")) {

    open();
}

OperationsLog::FileHeader::FileHeader():
    version(1),
    currentRecordNumber(0) {}

/*!
 * Returns SetOperation initialised from the received arguments.
 *
 * @param u1 - first node uuid (for example, first level node uuid in case of second level routing table)
 * @param u2 - second node uuid (for example, second level node uuid in case of second level routing table)
 * @param direction - direction between u1 and u2.
 *
 *
 * Throws RuntimeError in case if operation can't be written to the log.
 * Throws MemoryError.
 *
 *
 * Tests: OperationsLogTests::checkSetOperationsSerialization
 * (implicit tests)
 */
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

    } catch (Exception &e) {
        throw RuntimeError(
            string("OperationsLog::initSetOperation: can't initialise new transaction.")
            + e.message());
    }
}

/*!
 * Returns RemoveOperation initialised from the received arguments.
 *
 * @param u1 - first node uuid (for example, first level node uuid in case of second level routing table)
 * @param u2 - second node uuid (for example, second level node uuid in case of second level routing table)
 * @param direction - direction between u1 and u2.
 *
 *
 * Throws RuntimeError in case if operation can't be written to the log.
 * Throws MemoryError.
 *
 *
 * Tests: OperationsLogTests::checkRemoveOperationsSerialization
 * (implicit tests)
 */
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

    } catch (Exception &e) {
        throw RuntimeError(
            string("OperationsLog::initRemoveOperation: can't initialise new transaction.")
            + e.message());
    }
}

/*!
 * Returns DirectionUpdateOperation initialised from the received arguments.
 *
 * @param recN - record number of the record, that should be updated;
 * @param direction - direction between u1 and u2.
 * @param directionBackup - current direction between u1 and u2,
 * that would be restored in case if this operation would be rolled back.
 *
 *
 * Throws RuntimeError in case if operation can't be written to the log.
 * Throws MemoryError.
 *
 *
 * Tests: OperationsLogTests::checkDirectionUpdateOperationsSerialization
 * (implicit tests)
 */
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

    } catch (Exception &e) {
        throw RuntimeError(
            string("OperationsLog::initDirectionUpdateOperation: can't initialise new transaction.")
            + e.message());
    }
}

void OperationsLog::open() {
    db::AbstractFileDescriptorHandler::open();
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
 *
 *
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

/*!
 * Atomically writes the operation to the log.
 *
 *
 * Throws IOError in case when operation can't be saved to the disk.
 */
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
 * Returns operations, that was not commited.
 * Returns empty vector in case when no uncompleted operations are in the log.
 *
 * This method is usually called in case, when uncompleted operations must be rolled back,
 * so the operations are returned in reverse order.
 *
 *
 * Throws IOError in case when operations can't be loaded from the disk.
 * Throws RuntimeError in case when loaded operations can't be deserialized (file is corrupted).
 * Throws MemoryError.
 */
shared_ptr<vector<Operation::ConstShared>> OperationsLog::uncompletedOperations(){
    const size_t operationsDataSize = fileSize() - sizeof(FileHeader);
    if (operationsDataSize == 0){
        return shared_ptr<vector<Operation::ConstShared>>(new vector<Operation::ConstShared>());
    }

    auto operationsDataBuffer = (byte*)malloc(operationsDataSize);
    if (operationsDataBuffer == nullptr) {
        throw MemoryError(
            "OperationsLog::uncompletedOperations: bad allocation.");
    }
    // Operations buffer should be automatically freed at the function exit.
    shared_ptr<byte> operationsDataBufferHandler(operationsDataBuffer, free);


    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);
    if (fread(operationsDataBuffer, operationsDataSize, 1, mFileDescriptor) != 1 &&
        fread(operationsDataBuffer, operationsDataSize, 1, mFileDescriptor) != 1) {
        throw IOError(
            "OperationsLog::uncompletedOperations: "
                "can't read operations info from the disk.");
    }

    try {
        // note: bad_alloc is handled further.
        auto operations = shared_ptr<vector<Operation::ConstShared>>(new vector<Operation::ConstShared>);
        auto currentDataBufferOffset = operationsDataBuffer;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedValue"
        while (currentDataBufferOffset < (operationsDataBuffer + operationsDataSize)) {

            Operation::SerializedOperationType *operationType = currentDataBufferOffset;
            switch (*operationType) {
                case Operation::Set: {
                    // note: bad_alloc is handled further.
                    auto operation = Operation::ConstShared(new SetOperation(currentDataBufferOffset));

                    // Insert at the beginning (implicit reverse)
                    operations->insert(operations->cbegin(), operation);
                    currentDataBufferOffset += SetOperation::kSerializedSize;
                    break;
                }

                case Operation::Remove: {
                    // note: bad_alloc is handled further.
                    auto operation = Operation::ConstShared(new RemoveOperation(currentDataBufferOffset));

                    // Insert at the beginning (implicit reverse)
                    operations->insert(operations->cbegin(), operation);
                    currentDataBufferOffset += RemoveOperation::kSerializedSize;
                    break;
                }

                case Operation::Update: {
                    // note: bad_alloc is handled further.
                    auto operation = Operation::ConstShared(new DirectionUpdateOperation(currentDataBufferOffset));

                    // Insert at the beginning (implicit reverse)
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
#pragma clang diagnostic pop
        return operations;


    } catch (std::bad_alloc&) {
        throw MemoryError(
            "OperationsLog::uncompletedOperations: "
                "bad allocation.");
    }
}

/*
 * Removes all operations from the log.
 */
void OperationsLog::commit() {
    truncateOperations();
}

/*
 * Returns true if new transaction (set of operations) may be started.
 *
 * New transaction may be started only in case if file doesn't contains any operations,
 * otherwise - it can break the consistency.
 */
bool OperationsLog::transactionMayBeStarted() {
    return fileSize() == sizeof(FileHeader);
}

void OperationsLog::truncateOperations() {
    ftruncate(fileno(mFileDescriptor), sizeof(FileHeader));
}


} // namespace routing_tables;
} // namespace io;


