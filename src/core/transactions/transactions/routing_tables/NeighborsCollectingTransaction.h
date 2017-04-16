#ifndef GEO_NETWORK_CLIENT_NEIGHBORSCOLLECTINGTRANSACTION_H
#define GEO_NETWORK_CLIENT_NEIGHBORSCOLLECTINGTRANSACTION_H

#include "../base/BaseTransaction.h"

#include "../../../io/storage/RoutingTableHandler.h"
#include "../../../network/messages/routing_tables/NeighborsRequestMessage.h"


/**
 * This transaction asks remote node in the network about it's neighbors
 * and stores information about them into internal routing tables.
 *
 * In case if this operation is launched on the node, that changed (or removed) it's trust line
 * (nodes A and B on the scheme) - it also may init several subsidiary transactions,
 * to collect information about further nodes in the network (nodes B(2,n) or nodes A(2, n) if launched on the node B).
 *
 *
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
 *
 */
class NeighborsCollectingTransaction:
    public BaseTransaction {

public:
    NeighborsCollectingTransaction(
        const NodeUUID &destinationNode,
        const uint8_t hopDistance,
        RoutingTableHandler *routingTable2Level,
        RoutingTableHandler *routingTable3Level,
        Logger *logger)
        noexcept;

    TransactionResult::SharedConst run ()
        noexcept;

protected:
    TransactionResult::SharedConst processNeighborsRequestSending ()
        noexcept;

    TransactionResult::SharedConst processReceivedNeighborsInfo ()
        noexcept;

    void processNeighbors (
        const vector<NodeUUID> &neighbors)
        noexcept;

    void populateSecondLevelRoutingTable (
        const vector<NodeUUID> &neighbors,
        const NodeUUID &source)
        noexcept;

    void populateThirdLevelRoutingTable (
        const vector<NodeUUID> &neighbors,
        const NodeUUID &source)
        noexcept;

    void spawnChildNeighborsScanningTransactions(
        const vector<NodeUUID> &neighbors)
        noexcept;

protected:
    enum Stages {
        NeighborsRequestSending,
        NeighborsInfoProcessing
    };

    /*
     * UUID of the node, that must be asked for neighbors info.
     */
    const NodeUUID mDestinationNodeUUID;

    /*
     * Intermediate nodes count between the node that created or updated it's trust line,
     * and current node.
     */
    const uint8_t mHopDistance;

    /*
     * In case if transaction is launched on the node A or node B from the scheme,
     * 2 levels of neighbors must be pulled.
     *
     * This variable is used to control the recursion inside this transaction.
     * See uses of this variable for the details.
     */
    bool mSecondLevelNeighborsMustAlsoBeScanned;

    RoutingTableHandler *mRoutingTable2Level;
    RoutingTableHandler *mRoutingTable3Level;
};


#endif //GEO_NETWORK_CLIENT_NEIGHBORSCOLLECTINGTRANSACTION_H
