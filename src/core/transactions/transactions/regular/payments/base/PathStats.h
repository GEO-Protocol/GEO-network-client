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
        const Path::Shared path)
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

    const Path::Shared path () const
        noexcept;

    bool containsIntermediateNodes() const;

    const pair<BaseAddress::Shared, SerializedPositionInPath> currentIntermediateNodeAndPos () const
        throw (NotFoundError);

    const pair<BaseAddress::Shared, SerializedPositionInPath> nextIntermediateNodeAndPos () const
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
    const Path::Shared mPath;
    TrustLineAmount mMaxPathFlow;
    bool mIsValid;

    // Contains states of each node in the path.
    // See reservations stage for the details.
    vector<NodeState> mIntermediateNodesStates;
};


#endif //GEO_NETWORK_CLIENT_PATHSTATS_H
