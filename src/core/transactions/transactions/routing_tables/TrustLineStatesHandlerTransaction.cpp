#include "TrustLineStatesHandlerTransaction.h"


/**
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
 *
 *
 * @param neighborSenderUUID - UUID of the node, that propagated the notification about updated or removed trust line.
 *      For example, if trust line A-B was modified, and this transaction is launched on the node A1 -
 *      then this param would contains UUID of the node A. In case if this transaction is launched on the node A -
 *      then this param would also contains UUID of the node B.
 *
 * @param leftContractorUUID - UUID of the left node in pair of nodes, that has been changed (or removed) their trust line.
 *      For example, if trust line A-B was modified, and this transaction is launched on the node A -
 *      then this param would contains UUID of the node A.
 *
 * @param rightContractorUUID - UUID of the right node in pair of nodes, that has been changed (or removed) their trust line.
 *      For example, if trust line A-B was modified, and this transaction is launched on the node A -
 *      then this param would contains UUID of the node B.
 *
 * @param trustLineState - specifies what kind of change was done with the trust line.
 *      This parameter specifies the logic of this transaction:
 *      in case if trust line was created - it would try to collect info about neighbor nodes and populate routing tables,
 *      otherwise - it would update routing tables and remove obsolete records.
 *
 * @param hopDistance - count of middleware nodes between current node and the @conrtactorNode.
 *      For example, if trust line A-B was modified, and this transaction is launched on the node A -
 *      then this param would be set to 0. In case, if this transaction was launched on the node A1 -
 *      then this param would be set to 1, and so on.
 */
TrustLineStatesHandlerTransaction::TrustLineStatesHandlerTransaction (
    const NodeUUID &nodeUUID,
    const NodeUUID &neighborSenderUUID,
    const NodeUUID &leftContractorUUID,
    const NodeUUID &rightContractorUUID,
    const TrustLineState trustLineState,
    const uint8_t hopDistance,
    TrustLinesManager *trustLines,
    StorageHandler *storageHandler,
    Logger *logger)
    noexcept :

    BaseTransaction(
        BaseTransaction::RoutingTables_TrustLineStatesHandler,
        nodeUUID,
        logger),
    mLeftContractorUUID(leftContractorUUID),
    mRightContractorUUID(rightContractorUUID),
    mTrustLineState(trustLineState),
    mNeighborSenderUUID(neighborSenderUUID),
    mCurrentHopDistance(hopDistance),
    mTrustLines(trustLines),
    mStorageHandler(storageHandler)
{}

TransactionResult::SharedConst TrustLineStatesHandlerTransaction::run ()
    noexcept
{
    info() << "run on " << to_string(mCurrentHopDistance) << " level from " << mNeighborSenderUUID << " node";
    switch (mTrustLineState) {
        case Created:
            return processTrustLineCreation();
        case Removed:
            return processTrustLineRemoving();
        default:
            throw ValueError("TrustLineStatesHandlerTransaction::run: "
                                 "unexpected Trust line state");
    }
}

/*
 * Sends notification about updated trust line to the neighbor nodes (except newly added contractor).
 * Removes obsolete trust line from the routing tables.
 */
TransactionResult::SharedConst TrustLineStatesHandlerTransaction::processTrustLineRemoving()
    noexcept
{
    info() << "processTrustLineRemoving";
    if (mCurrentHopDistance < kRoutingTablesMaxLevel) {
        // Neighbor nodes must be notified about the newly added trust line.

        // ToDo: in case if notification message would be lost - there is nothing critical here:
        // ToDo: routing tables would be automatically updated on the next planned neighbors processing, but
        // ToDo: it would be much better if the neighbor node would be notified as quickly as possible,
        // ToDo: because, until it would be notified it would not be able to process any payment operations
        // ToDo: towards nodes, that has been created the trust line.

        const auto kNotificationMessage = make_shared<NotificationTrustLineRemovedMessage>(
            currentNodeUUID(),
            mLeftContractorUUID,
            mRightContractorUUID,
            mCurrentHopDistance+1);

        for (const auto kNeighborAndTL : mTrustLines->trustLines()) {
            sendMessageAndPreventRecursion(
                kNeighborAndTL.first,
                kNotificationMessage);
        }
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (mCurrentHopDistance == 0) {
        // Transaction is executed on the node A or node B of the scheme.
        // In this case, second level routing table must be cleared from records
        // that have source set to the counter node.
        //
        // For example, in case if transaction is launched on the node A,
        // then it's second level routing table must cleared from the records,
        // that has node B as a source.
        // Also, cascade removing of the related records of the third level routing table is needed.

        for (const auto kNeighborOfCounterpartNode : ioTransaction->routingTablesHandler()->neighborsOfOnRT2(mRightContractorUUID)) {
            removeRecordFromSecondLevel(
                ioTransaction,
                kNeighborOfCounterpartNode,
                mRightContractorUUID);
        }

        return resultDone();


    } else if (mCurrentHopDistance == 1) {
        // Transaction is executed on the some of nodes A1 or some of nodes B1 of the scheme.
        // In this case, only one record must be removed from the second level routing table:
        // A-B.
        removeRecordFromSecondLevel(
            ioTransaction,
            mNeighborSenderUUID,
            mRightContractorUUID);

        return resultDone();


    } else if (mCurrentHopDistance == 2) {
        // Transaction is executed on the some of the nodes A2 or on some of the nodes B2 of the scheme.
        // In this case, only one record must be removed from third level routing table.
        removeRecordFromThirdLevel(
            ioTransaction,
            mLeftContractorUUID,
            mRightContractorUUID);

        return resultDone();
    }


    throw RuntimeError(
        "TrustLineStatesHandlerTransaction::processTrustLineRemoving: "
            "invalid 'mCurrentHopDistance' occurred.");
}

/*
 * Sends notification about newly created trust line to the neighbor nodes (except newly added contractor).
 * Collects neighbor info from newly added contractor and populates routing tables with info about it's neighbor nodes.
 */
TransactionResult::SharedConst TrustLineStatesHandlerTransaction::processTrustLineCreation ()
    noexcept
{
    info() << "processTrustLineCreation";
    if (mCurrentHopDistance < kRoutingTablesMaxLevel) {
        // Neighbor nodes must be notified about the newly added trust line.

        // ToDo: in case if notification message would be lost - there is nothing critical here:
        // ToDo: routing tables would be automatically updated on the next planned neighbors processing, but
        // ToDo: it would be much better if the neighbor node would be notified as quickly as possible,
        // ToDo: because, until it would be notified it would not be able to process any payment operations
        // ToDo: towards nodes, that has been created the trust line.

        const auto kNotification = make_shared<NotificationTrustLineCreatedMessage>(
            currentNodeUUID(),
            mLeftContractorUUID,
            mRightContractorUUID,
            mCurrentHopDistance+1);

        for (const auto kNeighborAndTL : mTrustLines->trustLines()) {
            sendMessageAndPreventRecursion(
                kNeighborAndTL.first,
                kNotification);
        }
    }

    auto ioTransaction = mStorageHandler->beginTransaction();
    if (mCurrentHopDistance == 0) {
        info() << "0 hop distance level";
        // Transaction is executed on the node A or node B of the scheme.
        //
        // In this case neighbors of the node B must be collected,
        // then written to the 2-d level routing table,
        // and then - second level neighbors must be collected,
        // and written to tne 3-th level routing table.

        const auto kTransaction = make_shared<NeighborsCollectingTransaction>(
            currentNodeUUID(),
            mRightContractorUUID,
            currentNodeUUID(),
            mCurrentHopDistance,
            mStorageHandler,
            mLog);

        launchSubsidiaryTransaction(kTransaction);
        return resultDone();
    }

    if (mCurrentHopDistance == 1) {
        info() << "1 hop distance level";
        // Transaction is executed on the one of A1 nodes, or one of B1 nodes.
        //
        // In this case neighbors of the node B must be collected,
        // then written to the 3-d level routing table.

        writeRecordToSecondLevel(
            ioTransaction,
            mLeftContractorUUID,
            mRightContractorUUID);
        const auto kTransaction = make_shared<NeighborsCollectingTransaction>(
            currentNodeUUID(),
            mRightContractorUUID,
            mNeighborSenderUUID,
            mCurrentHopDistance,
            mStorageHandler,
            mLog);

        launchSubsidiaryTransaction(kTransaction);
        return resultDone();
    }

    if (mCurrentHopDistance == 2) {
        info() << "2 hop distance level";
        // Transaction is executed on the one of A2 nodes, or one of B2 nodes.
        //
        // No additional neighbors requests are needed.
        // Only node B must be written to the 3th level routing table.

        writeRecordToThirdLevel(
            ioTransaction,
            mLeftContractorUUID,
            mRightContractorUUID);

        return resultDone();
    }

    throw RuntimeError(
        "TrustLineStatesHandlerTransaction::processTrustLineCreation: "
            "invalid 'mCurrentHopDistance' occurred.");
}

/**
 * Sends @message to the @addressee, but prevents propagation to the 2 types of nodes:
 *
 * 1 - nodes, that has changed the trust line (nodes A and B in the scheme).
 * 2 - node, that has sent notification to the current node (neighbor node).
 */
void TrustLineStatesHandlerTransaction::sendMessageAndPreventRecursion (
    const NodeUUID &addressee,
    const Message::Shared message)
    noexcept
{
    if ((addressee == mNeighborSenderUUID) or
        (addressee == mLeftContractorUUID) or
        (addressee == mRightContractorUUID))
        return;

    sendMessage(
        addressee,
        message);
    info() << "send notification message to " << addressee;

    // ToDo: check if remote node received the notification.
}


/**
 * Removes (@source -> @destination) record from second level routing table.
 * Cascade removing of related records from third level routing table is done via the internal
 * logic of second level routing table handler.
 *
 * WARN: this method does'nt commits changes.
 */
void TrustLineStatesHandlerTransaction::removeRecordFromSecondLevel (
    IOTransaction::Shared ioTransaction,
    const NodeUUID &source,
    const NodeUUID &destination)
    noexcept
{
    try {
        ioTransaction->routingTablesHandler()->removeRecordFromRT2(
            source,
            destination);
    } catch (IOError &) {}
}

/**
 * Removes (@source -> @destination) record from second level routing table.
 *
 * WARN: this method does'nt commits changes.
 */
void TrustLineStatesHandlerTransaction::removeRecordFromThirdLevel (
    IOTransaction::Shared ioTransaction,
    const NodeUUID &source,
    const NodeUUID &destination)
    noexcept
{
    try {
        ioTransaction->routingTablesHandler()->removeRecordFromRT3(
            source,
            destination);
    } catch (IOError &) {}
}

/**
 * Inserts or updates (@source -> @destination) record to the second level routing table.
 */
void TrustLineStatesHandlerTransaction::writeRecordToSecondLevel (
    IOTransaction::Shared ioTransaction,
    const NodeUUID &source,
    const NodeUUID &destination)
throw (IOError)
{
    ioTransaction->routingTablesHandler()->setRecordToRT2(
        source,
        destination);
    debug() << "Record (" << source << " - " << destination << ") has been written to the RT2.";
}

/**
 * Inserts or updates (@source -> @destination) record to the third level routing table.
 */
void TrustLineStatesHandlerTransaction::writeRecordToThirdLevel (
    IOTransaction::Shared ioTransaction,
    const NodeUUID &source,
    const NodeUUID &destination)
    throw (IOError)
{
    ioTransaction->routingTablesHandler()->setRecordToRT3(
        source,
        destination);
    debug() << "Record (" << source << " - " << destination << ") has been written to the RT3.";
}

const string TrustLineStatesHandlerTransaction::logHeader() const
{
    stringstream s;
    s << "[TrustLineStatesHandlerTA: " << currentTransactionUUID() << "]";
    return s.str();
}