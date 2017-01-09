#include "Transaction.h"


namespace io {
namespace routing_tables {


Transaction::Transaction(
    const AbstractRoutingTable *routingTable):
    mRoutingTable(const_cast<AbstractRoutingTable*>(routingTable)),
    mState(Normal){}

Transaction::~Transaction() {
    if (mState == Normal) {
        commit();

    } else {
        rollback();
    }
}

void Transaction::set(
    const NodeUUID &u1,
    const NodeUUID &u2,
    const TrustLineDirection direction) {

    if (mState != Normal) {
        throw ConflictError(
            "Transaction::set: "
                "currnt transaction can't be used any more and would be rolled back.");
    }

    try {
        mRoutingTable->set(u1, u2, direction, false);
    } catch (Exception &e) {
        mState = Broken;

        // Re-throw the error to inform the developer about error.
        throw RuntimeError(
            std::string("Transaction::set: operation failed. Details are: ")
            + e.message());
    }
}

void Transaction::remove(
    const NodeUUID &u1,
    const NodeUUID &u2) {

    if (mState != Normal) {
        throw ConflictError(
            "Transaction::set: "
                "currnt transaction can't be used any more and would be rolled back.");
    }

    try {
        mRoutingTable->remove(u1, u2);
    } catch (Exception &e) {
        mState = Broken;

        // Re-throw the error to inform the developer about error.
        throw RuntimeError(
            std::string("Transaction::set: operation failed. Details are: ")
            + e.message());
    }
}

void Transaction::commit() {
    mRoutingTable->commitOperations();
}

void Transaction::rollback() {
    mRoutingTable->rollBackOperations();
}


}
}



