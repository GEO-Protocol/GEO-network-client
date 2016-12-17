#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H

#include "../../../db/fields/common/AbstractRecordsHandler.h"
#include "../../../db/fields/common/AbstractFileDescriptorHandler.h"
#include "../../../db/fields/tl_direction_column/TrustLineDirectionColumn.h"
#include "../../../common/NodeUUID.h"
#include "../../../common/exceptions/MemoryError.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../common/exceptions/ConflictError.h"
#include "../../../common/exceptions/NotFoundError.h"

#include <unistd.h>
#include <utility>
#include <malloc.h>


namespace io {
namespace routing_tables {


using namespace std;
using namespace db;
using namespace db::fields;


class BaseTransaction:
    public AbstractRecordsHandler {

public:
    enum TransactionType {
        Base              = 0,
        DirectionUpdating = 1,
        RecordInserting   = 2,
        RecordRemoving    = 3,
    };
    static const TransactionType type = Base;

    const RecordNumber recordNumber;

public:
    explicit BaseTransaction(
        const RecordNumber recordNumber);

    virtual const pair<void*, size_t> serialize() const = 0;

protected:
    static const size_t kTransactionTypeSize = 1;
    static const size_t kTLDirectionSize = 1;
};


class DirectionUpdateTransaction:
    public BaseTransaction {

public:
    static const TransactionType type = DirectionUpdating;

public:
    const TrustLineDirectionColumn::Direction direction;

public:
    explicit DirectionUpdateTransaction(
        const RecordNumber recN,
        const TrustLineDirectionColumn::Direction direction);

    explicit DirectionUpdateTransaction(
        const byte* serializedData);

    virtual const pair<void*, size_t> serialize() const;
};


class InsertTransaction:
    public BaseTransaction {

public:
    static const TransactionType type = RecordInserting;

public:
    const NodeUUID u1;
    const NodeUUID u2;
    const TrustLineDirectionColumn::Direction direction;

public:
    explicit InsertTransaction(
        const RecordNumber recN,
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirectionColumn::Direction direction);

    explicit InsertTransaction(
        const byte* serializedData);

    const pair<void*, size_t> serialize() const;
};


/*!
 * RemoveTransaction inherits from InsertTransaction
 * to be able to revert remove operation.
 *
 * Remove operation may be processed with u1 and u2 fields only.
 * Direction is storing to be able to revert this operation via
 * the InsertTransaction.
 */
class RemoveTransaction:
    public InsertTransaction {

public:
    static const TransactionType type = RecordRemoving;

public:
    explicit RemoveTransaction(
        const RecordNumber recN,
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirectionColumn::Direction direction);

    explicit RemoveTransaction(
        const byte* serializedData);
};


class TransactionsHandler:
    protected AbstractFileDescriptorHandler,
    protected AbstractRecordsHandler {

public:
    TransactionsHandler(
        const char *filename,
        const char *path);

    const InsertTransaction* beginInsertTransaction(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirectionColumn::Direction direction);

    const RemoveTransaction *beginRemoveTransaction(const NodeUUID &u1, const NodeUUID &u2,
                                                        const TrustLineDirectionColumn::Direction direction,
                                                        const RecordNumber recN);

    const DirectionUpdateTransaction* beginDirectionUpdateTransaction(
        const RecordNumber recN,
        const TrustLineDirectionColumn::Direction direction);

    const BaseTransaction* lastStartedTransaction();
    void removeLastTransaction();
    void commitLastTransaction();

protected:
    struct FileHeader {
    public:
        RecordNumber currentRecordNumber;

    public:
        FileHeader();
    };

    // Transaction handler is responsible for storing next record number,
    // that is used for set operations.
    // Next record number is common for
    RecordNumber mCurrentRecordNumber;

protected:
    void open(
        const char *accessMode);

    FileHeader loadFileHeader() const;
    void updateFileHeader(
        const FileHeader *header) const;

    void saveTransaction(const BaseTransaction *transaction);
    BaseTransaction* loadLastTransaction() const;
};


} // namespace routing_tables;
} // namespace io;




#endif //GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H
