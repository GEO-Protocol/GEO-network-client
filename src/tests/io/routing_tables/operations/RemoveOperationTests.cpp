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
            RemoveOperation op(uuid1, uuid2, dir);
            auto serialized = op.serialize();

            SetOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Remove);
            assert(deserializedOp.u1() == uuid1);
            assert(deserializedOp.u2() == uuid2);
            assert(deserializedOp.direction() == dir);
        }
        {
            NodeUUID uuid1;
            NodeUUID uuid2;
            TrustLineDirection dir(Outgoing);
            RemoveOperation op(uuid1, uuid2, dir);
            auto serialized = op.serialize();

            SetOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Remove);
            assert(deserializedOp.u1() == uuid1);
            assert(deserializedOp.u2() == uuid2);
            assert(deserializedOp.direction() == dir);
        }
        {
            NodeUUID uuid1;
            NodeUUID uuid2;
            TrustLineDirection dir(Both);
            RemoveOperation op(uuid1, uuid2, dir);
            auto serialized = op.serialize();

            SetOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Remove);
            assert(deserializedOp.u1() == uuid1);
            assert(deserializedOp.u2() == uuid2);
            assert(deserializedOp.direction() == dir);
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
        RemoveOperation op(uuid1, uuid2, dir);
        auto rollbackOp = op.rollbackOperation();

        assert(rollbackOp->type() == Operation::RollbackRemove);
        assert(rollbackOp->u1() == uuid1);
        assert(rollbackOp->u2() == uuid2);
    }
};