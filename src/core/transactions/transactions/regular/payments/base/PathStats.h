#ifndef GEO_NETWORK_CLIENT_PATHSTATS_H
#define GEO_NETWORK_CLIENT_PATHSTATS_H

#include "../../../../../paths/lib/Path.h"
#include "../../../../../common/Types.h"
#include "../../../../../common/exceptions/ValueError.h"
#include "../../../../../common/exceptions/NotFoundError.h"

// Describes payment path, it's nodes states,
// and max flow through it.
class PathStats {

public:
    enum NodeState {
        ReservationRequestDoesntSent = 0,

        NeighbourReservationRequestSent,
        NeighbourReservationApproved,

        ReservationRequestSent,
        ReservationApproved,
        ReservationRejected,
    };

public:
    PathStats (
        const Path::ConstShared path)
    noexcept;

    void setNodeState (
        const SerializedPositionInPath positionInPath,
        const NodeState state)
    throw(ValueError);

    const TrustLineAmount &maxFlow () const
    noexcept;

    void shortageMaxFlow (
        const TrustLineAmount &kAmount)
    throw(ValueError);

    const Path::ConstShared path () const
    noexcept;

    const pair<NodeUUID, SerializedPositionInPath> currentIntermediateNodeAndPos () const
    throw (NotFoundError);

    const pair<NodeUUID, SerializedPositionInPath> nextIntermediateNodeAndPos () const
    throw (NotFoundError);

    const bool reservationRequestSentToAllNodes () const
    noexcept;

    const bool isNeighborAmountReserved () const
    noexcept;

    const bool isWaitingForNeighborReservationResponse () const
    noexcept;

    const bool isWaitingForNeighborReservationPropagationResponse () const
    noexcept;

    const bool isWaitingForReservationResponse () const
    noexcept;

    const bool isReadyToSendNextReservationRequest () const
    noexcept;

    const bool isLastIntermediateNodeProcessed () const
    noexcept;

    const bool isLastIntermediateNodeApproved() const
    noexcept;

    const bool isValid () const
    noexcept;

    void setUnusable ()
    noexcept;

protected:
    const Path::ConstShared mPath;
    TrustLineAmount mMaxPathFlow;
    bool mIsValid;

    // Contains states of each node in the path.
    // See reservations stage for the details.
    vector<NodeState> mIntermediateNodesStates;

};


#endif //GEO_NETWORK_CLIENT_PATHSTATS_H
