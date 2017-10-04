#include "../../../../core/io/routing_tables/internal/transactions/internal/DirectionUpdateOperation.h"


using namespace io::routing_tables;


class DirectionUpdateOperationsTests {
public:
    void run() {
        checkSerialization();
        checkDeserialization();
        checkRollbackOperationGeneration();
    }

    void checkSerialization() {
        {
            TrustLineDirection dir(Incoming);
            TrustLineDirection dirBackup(Outgoing);
            AbstractRecordsHandler::RecordNumber recN(1);
            DirectionUpdateOperation op(recN, dir, dirBackup);
            auto serialized = op.serialize();

            DirectionUpdateOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Update);
            assert(deserializedOp.direction() == dir);
            assert(deserializedOp.directionBackup() == dirBackup);
            assert(deserializedOp.recordNumber() == recN);
        }
        {
            TrustLineDirection dir(Outgoing);
            TrustLineDirection dirBackup(Incoming);
            AbstractRecordsHandler::RecordNumber recN(1);
            DirectionUpdateOperation op(recN, dir, dirBackup);
            auto serialized = op.serialize();

            DirectionUpdateOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Update);
            assert(deserializedOp.direction() == dir);
            assert(deserializedOp.directionBackup() == dirBackup);
            assert(deserializedOp.recordNumber() == recN);
        }
        {
            TrustLineDirection dir(Both);
            TrustLineDirection dirBackup(Outgoing);
            AbstractRecordsHandler::RecordNumber recN(1);
            DirectionUpdateOperation op(recN, dir, dirBackup);
            auto serialized = op.serialize();

            DirectionUpdateOperation deserializedOp(serialized.first.get());
            assert(deserializedOp.type() == Operation::Update);
            assert(deserializedOp.direction() == dir);
            assert(deserializedOp.directionBackup() == dirBackup);
            assert(deserializedOp.recordNumber() == recN);
        }
    }

    void checkDeserialization() {
        // see: OperationsLogTests.cpp.
        // In that tests implicit deserialization test logic is present.
    }

    void checkRollbackOperationGeneration() {
        TrustLineDirection dir(Both);
        TrustLineDirection dirBackup(Outgoing);
        AbstractRecordsHandler::RecordNumber recN(1);
        DirectionUpdateOperation op(recN, dir, dirBackup);
        auto rollbackOp = op.rollbackOperation();

        assert(rollbackOp->type() == Operation::RollbackUpdate);
        assert(rollbackOp->direction() == dirBackup);
        assert(rollbackOp->recordNumber() == recN);
    }
};