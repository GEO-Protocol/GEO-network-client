#include "PathStats.h"

PathStats::PathStats(
    const Path::Shared path):

    mPath(path),
    mIntermediateNodesStates(
        path->length(),
        ReservationRequestDoesntSent),
    mMaxPathFlow(0),
    mIsValid(true)
{}

/**
 * Increases node state to to the "state".
 *
 * @param positionInPath - position of the node, which state must be increased.
 * @param state - new state of the node that should be remembered.
 *
 * @throws ValueError in case if new state is less than previous one.
 */
void PathStats::setNodeState(
    const SerializedPositionInPath positionInPath,
    const PathStats::NodeState state)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    assert(positionInPath > 0);
    assert(positionInPath <= mIntermediateNodesStates.size());
#endif

    mIntermediateNodesStates[positionInPath] = state;
}

/**
 * @returns current max flow of the path,
 * that was calculated due to amount reservation process.
 */
const TrustLineAmount& PathStats::maxFlow() const
{
    return mMaxPathFlow;
}

/**
 * @param kAmount - new max flow of the path.
 *
 * @throws ValueError in case of attempt to increase path max flow.
 */
void PathStats::shortageMaxFlow(
    const TrustLineAmount& kAmount)
{
    if (mMaxPathFlow == 0) {
        mMaxPathFlow = kAmount;
    }

    else if (kAmount <= mMaxPathFlow) {
        mMaxPathFlow = kAmount;
    }

    else
        throw ValueError(
            "PathStats::setMaxFlow: "
                "attempt to increase max flow occurred.");
}

const Path::Shared PathStats::path () const
{
    return mPath;
}

bool PathStats::containsIntermediateNodes() const
{
    return !mIntermediateNodesStates.empty();
}

/**
 * @returns node uuid (and it's position in the path),
 * from which reservation response must be received.
 *
 * @throws NotFoundError in case if no currently processed node,
 * or last in the path is already processed.
 */
const pair<BaseAddress::Shared, SerializedPositionInPath> PathStats::currentIntermediateNodeAndPos () const
{
    for (SerializedPositionInPath idx = 0; idx < mIntermediateNodesStates.size(); ++idx)
        if (mIntermediateNodesStates[idx] != PathStats::ReservationApproved &&
            mIntermediateNodesStates[idx] != PathStats::ReservationRejected)
            return make_pair(mPath->intermediates()[idx], idx);

    throw NotFoundError(
        "PathStats::currentIntermediateNodeAndPos: "
            "no unprocessed nodes are left.");
}

/**
 * @returns node to which amount reservation request wasn't set yet.
 * Amount reservation request may be both "CoordinatorReservationRequest" and "IntermediateNodeReservationRequest".
 * (this method ensures both requests are sent to the node)
 *
 * Also, node position (relative to the source node) would be returned.
 *
 * @throws NotFoundError - in case if all nodes of this path are already processed.
 */
const pair<BaseAddress::Shared, SerializedPositionInPath> PathStats::nextIntermediateNodeAndPos() const
{
    for (SerializedPositionInPath idx = 0; idx < mIntermediateNodesStates.size(); ++idx) {
        if (0 == idx &&
            mIntermediateNodesStates[idx] == PathStats::NeighbourReservationApproved) {
            return make_pair(mPath->intermediates()[idx], idx);
        }

        if (mIntermediateNodesStates[idx] == PathStats::ReservationRequestDoesntSent) {
            return make_pair(mPath->intermediates()[idx], idx);
        }
    }

    throw NotFoundError(
        "PathStats::nextIntermediateNodeAndPos: "
            "no unprocessed nodes are left.");
}

const bool PathStats::reservationRequestSentToAllNodes() const
{
    return mIntermediateNodesStates.at(
        mIntermediateNodesStates.size()-1) != ReservationRequestDoesntSent;
}

const bool PathStats::isNeighborAmountReserved() const
{
    return mIntermediateNodesStates[0] == PathStats::NeighbourReservationApproved;
}

const bool PathStats::isWaitingForNeighborReservationResponse() const
{
    return mIntermediateNodesStates[0] == PathStats::NeighbourReservationRequestSent;
}

const bool PathStats::isWaitingForNeighborReservationPropagationResponse() const
{
    return mIntermediateNodesStates[0] == PathStats::ReservationRequestSent;
}

/**
 * @returns true if current path sent amount reservation request and
 * is now waiting for the response to it.
 */
const bool PathStats::isWaitingForReservationResponse() const
{
    if (mPath->length() == 1) {
        // Return
    }

    for (const auto& it: mIntermediateNodesStates)
        if (it == PathStats::ReservationRequestSent)
            return true;

    return false;
}

const bool PathStats::isReadyToSendNextReservationRequest() const
{
    return !isWaitingForReservationResponse() &&
           !isWaitingForNeighborReservationResponse() &&
           !isLastIntermediateNodeProcessed();
}

const bool PathStats::isLastIntermediateNodeProcessed() const
{
    return
            mIntermediateNodesStates[mIntermediateNodesStates.size()-1] !=
                    PathStats::ReservationRequestDoesntSent &&
            mIntermediateNodesStates[mIntermediateNodesStates.size()-1] !=
                    PathStats::NeighbourReservationRequestSent &&
            mIntermediateNodesStates[mIntermediateNodesStates.size()-1] !=
                    PathStats::NeighbourReservationApproved;
}

const bool PathStats::isLastIntermediateNodeApproved() const
{
    return mIntermediateNodesStates[mIntermediateNodesStates.size()-1] ==
                   PathStats::NeighbourReservationApproved ||
           mIntermediateNodesStates[mIntermediateNodesStates.size()-1] ==
                   PathStats::ReservationApproved;
}

const bool PathStats::isValid() const
{
    return mIsValid;
}

void PathStats::setUnusable()
{
    mIsValid = false;
    mMaxPathFlow = 0;
}