#include "../../../core/io/routing_tables/internal/transactions/OperationsLog.h"


using namespace io::routing_tables;
namespace f = boost::filesystem;

class OperationsLogTests {
public:
    f::path path() {
        return f::path("tests/io/operations_log/");
    }

    void clean() {
        f::remove_all(path());
    }

    void run() {
        checkLogFileCreation();
        checkSetOperationsSerialization();
        checkRemoveOperationsSerialization();

        clean();
    };

    void checkLogFileCreation() {
        clean();
        OperationsLog log(path());
        assert(f::exists(path()));
    }

    void checkSetOperationsSerialization() {
        clean();

        OperationsLog log(path());
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

        auto operations = log.uncompletedOperations();
        assert(operations->size() == 3);

        auto operation = dynamic_cast<SetOperation*>(operations->at(0).get());
        assert(operation->type() == Operation::Set);
        assert(operation->u1() == uuid1);
        assert(operation->u2() == uuid2);
        assert(operation->direction() == dir);

        auto operation2 = dynamic_cast<SetOperation*>(operations->at(1).get());
        assert(operation2->type() == Operation::Set);
        assert(operation2->u1() == uuid3);
        assert(operation2->u2() == uuid4);
        assert(operation2->direction() == dir2);

        auto operation3 = dynamic_cast<SetOperation*>(operations->at(2).get());
        assert(operation3->type() == Operation::Set);
        assert(operation3->u1() == uuid5);
        assert(operation3->u2() == uuid6);
        assert(operation3->direction() == dir3);
    }

    void checkRemoveOperationsSerialization() {
        clean();

        OperationsLog log(path());
        NodeUUID uuid1;
        NodeUUID uuid2;
        TrustLineDirection dir(Incoming);
        log.initRemoveOperation(uuid1, uuid2, dir);

        NodeUUID uuid3;
        NodeUUID uuid4;
        TrustLineDirection dir2(Outgoing);
        log.initRemoveOperation(uuid3, uuid4, dir2);

        NodeUUID uuid5;
        NodeUUID uuid6;
        TrustLineDirection dir3(Outgoing);
        log.initRemoveOperation(uuid5, uuid6, dir3);

        auto operations = log.uncompletedOperations();
        assert(operations->size() == 3);

        auto operation = dynamic_cast<RemoveOperation*>(operations->at(0).get());
        assert(operation->type() == Operation::Remove);
        assert(operation->u1() == uuid1);
        assert(operation->u2() == uuid2);
        assert(operation->direction() == dir);

        auto operation2 = dynamic_cast<RemoveOperation*>(operations->at(1).get());
        assert(operation2->type() == Operation::Remove);
        assert(operation2->u1() == uuid3);
        assert(operation2->u2() == uuid4);
        assert(operation2->direction() == dir2);

        auto operation3 = dynamic_cast<RemoveOperation*>(operations->at(2).get());
        assert(operation3->type() == Operation::Remove);
        assert(operation3->u1() == uuid5);
        assert(operation3->u2() == uuid6);
        assert(operation3->direction() == dir3);
    }

};
