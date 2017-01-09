#include "../../../core/io/routing_tables/internal/AbstractRoutingTable.h"


using namespace io::routing_tables;
namespace f = boost::filesystem;

class AbstractRoutingTableTests {
public:
    void run() {
        checkAtomicInsert();
        checkAtomicRemoving();
        checkOperationsRollingBack();

        clean();
    };

protected:
    // Testable class that allows access to private methods;
    class TAbstractRoutingTable:
        public AbstractRoutingTable {
        friend class AbstractRoutingTableTests;

    public:
        using AbstractRoutingTable::AbstractRoutingTable;
    };

    // Returns path to the directory,
    // into which all the tests must be done.
    const f::path path() const {
        return f::path("tests/io/routing_tables/");
    }

    void clean() {
        f::remove_all(path());
    }


    void checkAtomicInsert() {
        clean();

        NodeUUID u1, u2, u3, u4, u5, u6;

        {
            TAbstractRoutingTable t(path(), 4);
            auto transaction = t.beginTransaction();
            transaction->set(u1, u2, Incoming);
            transaction->set(u3, u4, Outgoing);
            transaction->set(u5, u6, Both);
            transaction->commit();
        }
        {
            TAbstractRoutingTable t(path(), 4);
            auto recN1 = t.intersectingRecordNumber(u1, u2);
            assert(t.mDirColumn->direction(recN1) == Incoming);
            assert(t.mDirColumn->incomingDirectedRecordsNumbers().size() == 1);

            auto recN2 = t.intersectingRecordNumber(u3, u4);
            assert(t.mDirColumn->direction(recN2) == Outgoing);
            assert(t.mDirColumn->outgoingDirectedRecordsNumbers().size() == 1);

            auto recN3 = t.intersectingRecordNumber(u5, u6);
            assert(t.mDirColumn->direction(recN3) == Both);
            assert(t.mDirColumn->bothDirectedRecordsNumbers().size() == 1);
        }
    }

    void checkAtomicRemoving() {
        clean();

        NodeUUID u1, u2, u3, u4, u5, u6;

        {
            TAbstractRoutingTable t(path(), 4);
            auto transaction = t.beginTransaction();
            transaction->set(u1, u2, Incoming);
            transaction->set(u3, u4, Outgoing);
            transaction->set(u5, u6, Both);
            transaction->commit();
        }

        {
            TAbstractRoutingTable t(path(), 4);
            auto transaction = t.beginTransaction();
            transaction->remove(u1, u2);
            transaction->remove(u3, u4);
            transaction->remove(u5, u6);
            transaction->commit();
        }

        {
            TAbstractRoutingTable t(path(), 4);
            try {
                t.intersectingRecordNumber(u1, u2);
                t.intersectingRecordNumber(u3, u4);
                t.intersectingRecordNumber(u5, u6);
                assert(false);

            } catch (IndexError &) {
                // ok
            }

            // Check dir column internal logic
            assert(t.mDirColumn->incomingDirectedRecordsNumbers().size() == 0);
            assert(t.mDirColumn->outgoingDirectedRecordsNumbers().size() == 0);
            assert(t.mDirColumn->bothDirectedRecordsNumbers().size() == 0);
        }
    }

    void checkOperationsRollingBack() {
        clean();

        NodeUUID u1, u2, u3, u4, u5, u6;

        {
            TAbstractRoutingTable t(path(), 4);
            auto transaction = t.beginTransaction();
            transaction->set(u1, u2, Incoming);
            transaction->set(u3, u4, Outgoing);
            transaction->set(u5, u6, Both);


            t.rollBackOperations();
        }
        {
            TAbstractRoutingTable t(path(), 4);
            try {
                t.intersectingRecordNumber(u1, u2);
                t.intersectingRecordNumber(u3, u4);
                t.intersectingRecordNumber(u5, u6);
                assert(false);

            } catch (IndexError &) {
                // ok
            }

            // Check dir column internal logic
            assert(t.mDirColumn->incomingDirectedRecordsNumbers().size() == 0);
            assert(t.mDirColumn->outgoingDirectedRecordsNumbers().size() == 0);
            assert(t.mDirColumn->bothDirectedRecordsNumbers().size() == 0);
        }
    }
};