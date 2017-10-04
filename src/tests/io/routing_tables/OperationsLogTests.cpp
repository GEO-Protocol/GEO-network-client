#include "../../../core/io/routing_tables/internal/transactions/OperationsLog.h"


using namespace io::routing_tables;
namespace f = boost::filesystem;


class OperationsLogTests {
public:
    // Testable class, that allows acces to the private methods and fields.
    class TOperationsLog:
        public OperationsLog {
        friend class OperationsLogTests;
    public:
        using OperationsLog::OperationsLog;
    };


    // Returns path to the directory,
    // into which all the tests must be done.
    const f::path path() const {
        return f::path("tests/io/routing_tables/operations_log/");
    }

    void clean() {
        f::remove_all(path());
    }

    void run() {
        checkLogFileCreation();
        checkSetOperationsSerialization();
        checkRemoveOperationsSerialization();
        checkDirectionUpdateOperationsSerialization();
        checkCommit();
        checkTruncateOperations();


        clean();
    };

    void checkLogFileCreation() {
        clean();
        TOperationsLog log(path());

        // Operations log should create file for storing transactions, that are in progress.
        assert(f::exists(log.path() / log.filename()));

        // Initial file size should be eaual to the file header size.
        assert(log.fileSize() == sizeof(TOperationsLog::FileHeader));
    }

    /*
     * This test tries to serialize and write several "SetOperation" transactions to the log.
     * All transactions should be written sequentially.
     */
    void checkSetOperationsSerialization() {
        clean();

        TOperationsLog log(path());

        NodeUUID uuid1;
        NodeUUID uuid2;
        TrustLineDirection dir(Incoming);
        log.initSetOperation(uuid1, uuid2, dir);

        NodeUUID uuid3;
        NodeUUID uuid4;
        TrustLineDirection dir2(Outgoing);
        log.initSetOperation(uuid3, uuid4, dir2);

        NodeUUID uuid5;
        NodeUUID uuid6;
        TrustLineDirection dir3(Outgoing);
        log.initSetOperation(uuid5, uuid6, dir3);

        // Check if log correctly reports 3 uncompleted operations.
        auto operations = log.uncompletedOperations();
        assert(operations->size() == 3);

        // Operations should be returned (and rolled back) in reverse order
        auto operation3 = dynamic_cast<const SetOperation*>(operations->at(0).get());
        assert(operation3->type() == Operation::Set);
        assert(operation3->u1() == uuid5);
        assert(operation3->u2() == uuid6);
        assert(operation3->direction() == dir3);

        auto operation2 = dynamic_cast<const SetOperation*>(operations->at(1).get());
        assert(operation2->type() == Operation::Set);
        assert(operation2->u1() == uuid3);
        assert(operation2->u2() == uuid4);
        assert(operation2->direction() == dir2);

        auto operation = dynamic_cast<const SetOperation*>(operations->at(2).get());
        assert(operation->type() == Operation::Set);
        assert(operation->u1() == uuid1);
        assert(operation->u2() == uuid2);
        assert(operation->direction() == dir);
    }

    void checkRemoveOperationsSerialization() {
        clean();

        OperationsLog log(path());
        NodeUUID uuid1;
        NodeUUID uuid2;
        TrustLineDirection dir(Incoming);
        AbstractRecordsHandler::RecordNumber recN1 = 100;
        log.initRemoveOperation(uuid1, uuid2, dir, recN1);

        NodeUUID uuid3;
        NodeUUID uuid4;
        TrustLineDirection dir2(Outgoing);
        AbstractRecordsHandler::RecordNumber recN2 = 200;
        log.initRemoveOperation(uuid3, uuid4, dir2, recN2);

        NodeUUID uuid5;
        NodeUUID uuid6;
        TrustLineDirection dir3(Both);
        AbstractRecordsHandler::RecordNumber recN3 = 300;
        log.initRemoveOperation(uuid5, uuid6, dir3, recN3);

        auto operations = log.uncompletedOperations();
        assert(operations->size() == 3);

        // Operations should be rolled back in reverse order
        auto operation3 = dynamic_cast<const RemoveOperation*>(operations->at(0).get());
        assert(operation3->type() == Operation::Remove);
        assert(operation3->u1() == uuid5);
        assert(operation3->u2() == uuid6);
        assert(operation3->direction() == dir3);
        assert(operation3->recordNumber() == recN3);

        auto operation2 = dynamic_cast<const RemoveOperation*>(operations->at(1).get());
        assert(operation2->type() == Operation::Remove);
        assert(operation2->u1() == uuid3);
        assert(operation2->u2() == uuid4);
        assert(operation2->direction() == dir2);
        assert(operation2->recordNumber() == recN2);

        auto operation = dynamic_cast<const RemoveOperation*>(operations->at(2).get());
        assert(operation->type() == Operation::Remove);
        assert(operation->u1() == uuid1);
        assert(operation->u2() == uuid2);
        assert(operation->direction() == dir);
        assert(operation->recordNumber() == recN1);
    }

    void checkDirectionUpdateOperationsSerialization() {
        clean();

        OperationsLog log(path());
        TrustLineDirection dir1(Incoming);
        TrustLineDirection dirBackup1(Outgoing);
        AbstractRecordsHandler::RecordNumber recN1(1);
        log.initDirectionUpdateOperation(recN1, dir1, dirBackup1);

        TrustLineDirection dir2(Outgoing);
        TrustLineDirection dirBackup2(Incoming);
        AbstractRecordsHandler::RecordNumber recN2(1);
        log.initDirectionUpdateOperation(recN2, dir2, dirBackup2);

        TrustLineDirection dir3(Incoming);
        TrustLineDirection dirBackup3(Outgoing);
        AbstractRecordsHandler::RecordNumber recN3(1);
        log.initDirectionUpdateOperation(recN3, dir3, dirBackup3);

        auto operations = log.uncompletedOperations();
        assert(operations->size() == 3);

        // operations should be rolled back in reverse order
        auto operation3 = dynamic_cast<const DirectionUpdateOperation*>(operations->at(0).get());
        assert(operation3->type() == Operation::Update);
        assert(operation3->direction() == dir3);
        assert(operation3->directionBackup() == dirBackup3);
        assert(operation3->recordNumber() == recN3);

        auto operation2 = dynamic_cast<const DirectionUpdateOperation*>(operations->at(1).get());
        assert(operation2->type() == Operation::Update);
        assert(operation2->direction() == dir2);
        assert(operation2->directionBackup() == dirBackup2);
        assert(operation2->recordNumber() == recN2);

        auto operation = dynamic_cast<const DirectionUpdateOperation*>(operations->at(2).get());
        assert(operation->type() == Operation::Update);
        assert(operation->direction() == dir1);
        assert(operation->directionBackup() == dirBackup1);
        assert(operation->recordNumber() == recN1);
    }

    /*
     * Commit operation should clear the log.
     * This is the same as truncate, seprate test cases is created only
     * to be able to extend it in the future, in case when commit operations will change the logic (if so).
     */
    void checkCommit() {
        clean();

        OperationsLog log(path());
        TrustLineDirection dir1(Incoming);
        TrustLineDirection dirBackup1(Outgoing);
        AbstractRecordsHandler::RecordNumber recN1(1);
        log.initDirectionUpdateOperation(recN1, dir1, dirBackup1);

        TrustLineDirection dir2(Outgoing);
        TrustLineDirection dirBackup2(Incoming);
        AbstractRecordsHandler::RecordNumber recN2(1);
        log.initDirectionUpdateOperation(recN2, dir2, dirBackup2);


        log.commit();
        assert(log.uncompletedOperations()->size() == 0);
    }

    /*
     * Truncate oprations should clear the log.
     */
    void checkTruncateOperations() {
        clean();

        OperationsLog log(path());
        TrustLineDirection dir1(Incoming);
        TrustLineDirection dirBackup1(Outgoing);
        AbstractRecordsHandler::RecordNumber recN1(1);
        log.initDirectionUpdateOperation(recN1, dir1, dirBackup1);

        TrustLineDirection dir2(Outgoing);
        TrustLineDirection dirBackup2(Incoming);
        AbstractRecordsHandler::RecordNumber recN2(1);
        log.initDirectionUpdateOperation(recN2, dir2, dirBackup2);


        log.truncateOperations();
        assert(log.uncompletedOperations()->size() == 0);
    }
};
