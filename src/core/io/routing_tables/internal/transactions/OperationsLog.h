#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H


#include "internal/SetOperation.h"
#include "internal/DirectionUpdateOperation.h"
#include "internal/RemoveOperation.h"

#include "../../../../db/fields/common/AbstractFileDescriptorHandler.h"
#include "../../../../common/exceptions/MemoryError.h"
#include "../../../../common/exceptions/ValueError.h"
#include "../../../../common/exceptions/ConflictError.h"
#include "../../../../common/exceptions/NotFoundError.h"
#include "../../../../common/exceptions/RuntimeError.h"

#include <unistd.h>
#include <utility>
#include <malloc.h>


namespace io {
namespace routing_tables {


using namespace std;
using namespace db;
namespace fs = boost::filesystem;


class OperationsLog:
    protected AbstractFileDescriptorHandler,
    protected AbstractRecordsHandler {

#ifdef TESTS__ROUTING_TABLE
    friend class OperationsLogTests;
#endif

public:
    OperationsLog(
        const fs::path &path);

    const SetOperation::Shared initSetOperation(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirection direction);

    const RemoveOperation::Shared initRemoveOperation(
        const NodeUUID &u1,
        const NodeUUID &u2,
        const TrustLineDirection directionBackup,
        const RecordNumber recN);

    const DirectionUpdateOperation::Shared initDirectionUpdateOperation(
        const RecordNumber recN,
        const TrustLineDirection direction,
        const TrustLineDirection directionBackup);

    const RecordNumber nextRecordNumber();

    bool transactionMayBeStarted();

    void commit();
    shared_ptr<vector<Operation::ConstShared>> uncompletedOperations();
    void truncateOperations();

protected:
    struct FileHeader {
    public:
        uint16_t version;
        RecordNumber currentRecordNumber;

    public:
        FileHeader();
    };

    // Transaction handler is responsible for storing next record number,
    // that is used for set operations.
    // Next record number is common for
    RecordNumber mCurrentRecordNumber;

protected:
    void open();
    FileHeader loadFileHeader() const;
    void updateFileHeader(
        const FileHeader *header) const;

    void logOperation(
        const Operation::Shared operation);
};


} // namespace routing_tables;
} // namespace io;

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSHANDLER_H
