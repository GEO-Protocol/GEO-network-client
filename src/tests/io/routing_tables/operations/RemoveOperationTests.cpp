#include "../../../../core/io/routing_tables/internal/transactions/internal/RemoveOperation.h"


using namespace io::routing_tables;

class RemoveOperationsTests {
public:
    void run() {
        checkSerialization();
        checkDeserialization();
        checkRollbackOperationGeneration();
    }

    void checkSerialization() {
        {
            NodeUUID uuid1;
            NodeUUID uuid2;
            TrustLineDirection dir(Incoming);
            AbstractRecordsHandler::RecordNumber recN = 100;
            RemoveOperation op(uuid1, uuid2, dir, recN);
            auto serialized = op.serialize();

            SetOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Remove);
            assert(deserializedOp.u1() == uuid1);
            assert(deserializedOp.u2() == uuid2);
            assert(deserializedOp.direction() == dir);
            assert(deserializedOp.recordNumber() == recN);
        }
        {
            NodeUUID uuid1;
            NodeUUID uuid2;
            TrustLineDirection dir(Outgoing);
            AbstractRecordsHandler::RecordNumber recN = 100;
            RemoveOperation op(uuid1, uuid2, dir, recN);
            auto serialized = op.serialize();

            SetOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Remove);
            assert(deserializedOp.u1() == uuid1);
            assert(deserializedOp.u2() == uuid2);
            assert(deserializedOp.direction() == dir);
            assert(deserializedOp.recordNumber() == recN);
        }
        {
            NodeUUID uuid1;
            NodeUUID uuid2;
            TrustLineDirection dir(Both);
            AbstractRecordsHandler::RecordNumber recN = 100;
            RemoveOperation op(uuid1, uuid2, dir, recN);

            auto serialized = op.serialize();

            SetOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Remove);
            assert(deserializedOp.u1() == uuid1);
            assert(deserializedOp.u2() == uuid2);
            assert(deserializedOp.direction() == dir);
            assert(deserializedOp.recordNumber() == recN);
        }
    }

    void checkDeserialization() {
        // see: OperationsLogTests.cpp.
        // In that tests implicit deserialization test logic is present.
    }

    void checkRollbackOperationGeneration() {
        NodeUUID uuid1;
        NodeUUID uuid2;
        TrustLineDirection dir(Incoming);
        AbstractRecordsHandler::RecordNumber recN = 100;
        RemoveOperation op(uuid1, uuid2, dir, recN);
        auto rollbackOp = op.rollbackOperation();

        assert(rollbackOp->type() == Operation::RollbackRemove);
        assert(rollbackOp->u1() == uuid1);
        assert(rollbackOp->u2() == uuid2);
    }
};