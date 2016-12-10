#include "TransactionsHandler.h"


namespace io {
namespace routing_tables {


BaseTransaction::BaseTransaction(
    const RecordNumber recordNumber):

    recordNumber(recordNumber) {}


DirectionUpdateTransaction::DirectionUpdateTransaction(
    const AbstractRecordsHandler::RecordNumber recN,
    const TrustLineDirectionColumn::Direction direction):

    BaseTransaction(recN),
    direction(direction) {}

DirectionUpdateTransaction::DirectionUpdateTransaction(
    const AbstractRecordsHandler::byte *serializedData):

    BaseTransaction(*serializedData),
    direction(*(serializedData + sizeof(RecordNumber))) {

#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(serializedData != nullptr);
#endif
}

const pair<void *, size_t> DirectionUpdateTransaction::serialize() const {
    const size_t kBufferSize =
        + kTransactionTypeSize
        + kTLDirectionSize;

    const size_t kTLDirectionOffset = kTransactionTypeSize;


    // Serializing
    auto *buffer = (byte*)malloc(kBufferSize);
    if (buffer == nullptr) {
        throw MemoryError(
            "DirectionUpdateTransaction::serialize: "
                "can't allocate enough memory for data block.");
    }

    memcpy(buffer, &type, kTransactionTypeSize);
    memcpy(buffer+kTLDirectionOffset, &direction, kTLDirectionSize);

    return make_pair(buffer, kBufferSize);

}


InsertTransaction::InsertTransaction(
    const AbstractRecordsHandler::RecordNumber recN,
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirectionColumn::Direction direction):

    BaseTransaction(recN),
    u1(u1),
    u2(u2),
    direction(direction){}

InsertTransaction::InsertTransaction(
    const AbstractRecordsHandler::byte *serializedData):

    BaseTransaction(*serializedData),
    u1(NodeUUID(serializedData + sizeof(RecordNumber))),
    u2(NodeUUID(serializedData + sizeof(RecordNumber) + 16)),
    direction(*(serializedData + sizeof(RecordNumber) + 32)) {}

const pair<void*, size_t> InsertTransaction::serialize() const {
    const size_t kBufferSize =
        + kTransactionTypeSize
        + NodeUUID::kUUIDLength
        + NodeUUID::kUUIDLength
        + kTLDirectionSize;

    const size_t kU1Offset = kTransactionTypeSize;
    const size_t kU2Offset = kU1Offset + NodeUUID::kUUIDLength;
    const size_t kDirectionOffset = kU2Offset + NodeUUID::kUUIDLength;


    // Serializing
    auto *buffer = (byte*)malloc(kBufferSize);
    if (buffer == nullptr) {
        throw MemoryError(
            "InsertTransaction::serialize: "
                "can't allocate enough memory for data block.");
    }

    memcpy(buffer, &type, kTransactionTypeSize);
    memcpy(buffer+kU1Offset, &u1, NodeUUID::kUUIDLength);
    memcpy(buffer+kU2Offset, &u2, NodeUUID::kUUIDLength);
    memcpy(buffer+kDirectionOffset, &direction, kTLDirectionSize);

    return make_pair(buffer, kBufferSize);
}


RemoveTransaction::RemoveTransaction(
    const RecordNumber recN,
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirectionColumn::Direction direction) :

    InsertTransaction(recN, u1, u2, direction) {}

RemoveTransaction::RemoveTransaction(
    const AbstractRecordsHandler::byte *serializedData):

    InsertTransaction(serializedData) {}


TransactionsHandler::TransactionsHandler(
    const char *filename,
    const char *path):

    AbstractFileDescriptorHandler(filename, path) {
    open(kWriteAccessMode);
}

TransactionsHandler::FileHeader::FileHeader():
    currentRecordNumber(0) {}

const InsertTransaction* TransactionsHandler::beginInsertTransaction(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirectionColumn::Direction direction) {

    try {
        auto transaction = new InsertTransaction(mCurrentRecordNumber, u1, u2, direction);
        saveTransaction(transaction);
        return transaction;

    } catch (...) {
        throw Exception(
            "TransactionsHandler::beginInsertTransaction: "
                "can't initialise new transaction.");
    }
}

const RemoveTransaction *TransactionsHandler::beginRemoveTransaction(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirectionColumn::Direction direction,
    const RecordNumber recN) {

    try {
        auto transaction = new RemoveTransaction(recN, u1, u2, direction);
        saveTransaction(transaction);
        return transaction;

    } catch (...) {
        throw Exception(
            "TransactionsHandler::beginRemoveTransaction: "
                "can't initialise new transaction.");
    }
}

const DirectionUpdateTransaction* TransactionsHandler::beginDirectionUpdateTransaction(
    const RecordNumber recN,
    const TrustLineDirectionColumn::Direction direction) {

    try {
        lastStartedTransaction();
        throw ConflictError(
            "TransactionsHandler::beginDirectionUpdateTransaction: "
                "last transaction was not committed or rolled back.");
    } catch (NotFoundError &) {}

    auto transaction = new DirectionUpdateTransaction(recN, direction);
    saveTransaction(transaction);
    return transaction;
}

const BaseTransaction *TransactionsHandler::lastStartedTransaction() {
    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);

    size_t dataBlockSize = 0;
    if (fread(&dataBlockSize, sizeof(dataBlockSize), 1, mFileDescriptor) != 1){
        throw IOError(
            "TransactionsHandler::lastStartedTransaction: "
                "can't read transaction data block size.");
    }

    if (dataBlockSize == 0){
        throw ValueError(
            "TransactionsHandler::lastStartedTransaction: "
                "transaction data block size can't be zero.");
    }

    // Load the transaction from the disk.
    auto buffer = (byte*)malloc(dataBlockSize);
    if (fread(&buffer, dataBlockSize, 1, mFileDescriptor) != 1){
        throw IOError(
            "TransactionsHandler::lastStartedTransaction: "
                "can't read transaction data block.");
    }

    // Transaction parsing.
    try {
        switch (buffer[0]){
            case BaseTransaction::RecordInserting: {
                auto transaction = new InsertTransaction(buffer+1);
                free(buffer);
                return transaction;
            }

            case BaseTransaction::RecordRemoving: {
                auto transaction = new RemoveTransaction(buffer+1);
                free(buffer);
                return transaction;
            }

            case BaseTransaction::DirectionUpdating: {
                auto transaction = new DirectionUpdateTransaction(buffer+1);
                free(buffer);
                return transaction;
            }

            default:
                free(buffer);
                throw ValueError(
                    "TransactionsHandler::lastStartedTransaction: "
                        "unexpected transaction type occurred.");
        }

    } catch (...) {
        throw ValueError(
            "TransactionsHandler::lastStartedTransaction: "
                "transaction data block can't be deserialised.");
    }
}

void TransactionsHandler::open(
    const char *accessMode) {

    db::AbstractFileDescriptorHandler::open(accessMode);
    if (fileSize() == 0) {
        // Init default header.
        FileHeader initialHeader; // will be initialised to the defaults by the constructor.
        updateFileHeader(initialHeader);
        mCurrentRecordNumber = initialHeader.currentRecordNumber;

    } else {
        // Load data from current header.
        auto fileHeader = loadFileHeader();
        mCurrentRecordNumber = fileHeader.currentRecordNumber;
    }
}

TransactionsHandler::FileHeader TransactionsHandler::loadFileHeader() const {
    FileHeader header;
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fread(&header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TransactionsHandler::loadFileHeader: "
                "can't load file header.");
    }
    return header;
}

/*!
 * Atomically updates file header.
 * Throws IOError in case when write operation failed.
 */
void TransactionsHandler::updateFileHeader(const TransactionsHandler::FileHeader &header) const {
    fseek(mFileDescriptor, 0, SEEK_SET);
    if (fwrite(header, sizeof(header), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TransactionsHandler::updateFileHeader: "
                "can't write header to the disk.");
    }
    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "TransactionsHandler::updateFileHeader: "
                "can't sync buffers with the disk.");
    }
}


void TransactionsHandler::saveTransaction(const BaseTransaction *transaction) {
    // seek to the file header end.
    fseek(mFileDescriptor, sizeof(FileHeader), SEEK_SET);

    auto serialisationPair = transaction->serialize();
    const size_t dataBytesCount = serialisationPair.second;
    const byte* data = (byte*)serialisationPair.first;

    if (fwrite(&dataBytesCount, sizeof(dataBytesCount), 1, mFileDescriptor) != 1) {
        throw IOError(
            "TransactionsHandler::saveTransaction: "
                "can't write transaction block size.");
    }

    if (fwrite(data, dataBytesCount, 1, mFileDescriptor) != 1) {
        throw IOError(
            "TransactionsHandler::saveTransaction: "
                "can't write transaction block.");
    }

    if (fdatasync(fileno(mFileDescriptor)) != 0) {
        throw IOError(
            "TransactionsHandler::saveTransaction: "
                "can't sync buffer with the device.");
    }
}
} // namespace routing_tables;
} // namespace io;


