#ifndef GEO_NETWORK_CLIENT_PATHSTATS_H
#define GEO_NETWORK_CLIENT_PATHSTATS_H

#include "../../../../../paths/lib/Path.h"
#include "../../../../../common/Types.h"
#include "../../../../../common/exceptions/ValueError.h"
#include "../../../../../common/exceptions/NotFoundError.h"

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
        const Path::Shared path);

    void setNodeState (
        const SerializedPositionInPath positionInPath,
        const NodeState state);

    const TrustLineAmount &maxFlow () const;

    void shortageMaxFlow (
        const TrustLineAmount &kAmount);

    const Path::Shared path () const;

    bool containsIntermediateNodes() const;

    const pair<BaseAddress::Shared, SerializedPositionInPath> currentIntermediateNodeAndPos () const;

    const pair<BaseAddress::Shared, SerializedPositionInPath> nextIntermediateNodeAndPos () const;

    const bool reservationRequestSentToAllNodes () const;

    const bool isNeighborAmountReserved () const;

    const bool isWaitingForNeighborReservationResponse () const;

    const bool isWaitingForNeighborReservationPropagationResponse () const;

    const bool isWaitingForReservationResponse () const;

    const bool isReadyToSendNextReservationRequest () const;

    const bool isLastIntermediateNodeProcessed () const;

    const bool isLastIntermediateNodeApproved() const;

    const bool isValid () const;

    void setUnusable ();

protected:
    const Path::Shared mPath;
    TrustLineAmount mMaxPathFlow;
    bool mIsValid;

    // Contains states of each node in the path.
    // See reservations stage for the details.
    vector<NodeState> mIntermediateNodesStates;
};


#endif //GEO_NETWORK_CLIENT_PATHSTATS_H
