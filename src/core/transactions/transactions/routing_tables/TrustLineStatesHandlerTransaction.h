#ifndef GEO_NETWORK_CLIENT_TRUSTLINESTATESHANDLERTRANSACTION_H
#define GEO_NETWORK_CLIENT_TRUSTLINESTATESHANDLERTRANSACTION_H

#include "NeighborsCollectingTransaction.h"

#include "../../../common/Constraints.h"
#include "../../../io/storage/RoutingTablesHandler.h"
#include "../../../trust_lines/manager/TrustLinesManager.h"

#include "../../../network/messages/routing_tables/NotificationTrustLineRemovedMessage.h"
#include "../../../network/messages/routing_tables/NotificationTrustLineCreatedMessage.h"


/*
 * This transaction handles trust line creation (or removing) events
 * and tries to update routing tables through the child transactions.
 */
class TrustLineStatesHandlerTransaction:
    public BaseTransaction {

public:
    enum TrustLineState {
        Created,
        Removed,
    };

public:
    TrustLineStatesHandlerTransaction(
        const NodeUUID &neighborSenderUUID,
        const NodeUUID &leftContractorUUID,
        const NodeUUID &rightContractorUUID,
        const TrustLineState trustLineState,
        const uint8_t hopDistance,

        TrustLinesManager *trustLines,

        RoutingTablesHandler *routingTables,
        Logger *logger)
        noexcept;

    TransactionResult::SharedConst run()
        noexcept;

protected:
    TransactionResult::SharedConst processTrustLineRemoving()
        noexcept;

    TransactionResult::SharedConst processTrustLineCreation ()
        noexcept;

protected:
    void removeRecordFromSecondLevel (
        const NodeUUID &source,
        const NodeUUID &destination)
        noexcept;

    void removeRecordFromThirdLevel (
        const NodeUUID &source,
        const NodeUUID &destination)
        noexcept;

    void writeRecordToThirdLevel (
        const NodeUUID &source,
        const NodeUUID &destination)
        throw (IOError);

    void sendMessageAndPreventRecursion(
        const NodeUUID &addressee,
        const Message::Shared message)
        noexcept;

protected:
    /*
     * Scheme:
     *
     *   A(2,1) --+                                            +-- B (2,1)
     *            |                                            |
     *            +-- A(1,1) ---+               +--- B (1,1) --+
     *            |             |               |              |
     *   A(2,2) --+             |               |              +-- B (2,2)
     *                          +--- A === B ---+
     *   A(2,3) --+             |               |              +-- B (2,3)
     *            |             |               |              |
     *            +-- A(1,2) ---+               +--- B (1,2) --+
     *            |                                            |
     *   A(2,4) --+                                            +-- B (2,4)
     */

    const NodeUUID mLeftContractorUUID;     // Node A in the scheme
    const NodeUUID mRightContractorUUID;    // Node B in the scheme

    const TrustLineState mTrustLineState;
    const NodeUUID mNeighborSenderUUID;     // UUID of the node that has been sent the notification
    const uint8_t mCurrentHopDistance;

    TrustLinesManager *mTrustLines;
    RoutingTablesHandler *mRoutingTables;
};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESTATESHANDLERTRANSACTION_H
