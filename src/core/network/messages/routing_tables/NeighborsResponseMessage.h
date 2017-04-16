#ifndef GEO_NETWORK_CLIENT_NEIGHBORSRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_NEIGHBORSRESPONSEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

#include "../../../common/Constraints.h"
#include "../../../common/exceptions/OverflowError.h"

#include <vector>


/*
 * This message is used to transfer first level nodes to the remote node.
 * This list is used for populating routing tables on the remote node.
 */
class NeighborsResponseMessage:
    public TransactionMessage {

public:
    NeighborsResponseMessage(
        const NodeUUID &senderUIID,
        const TransactionUUID &transactionUUID,
        const uint16_t expectedNodesCount=0)
        noexcept;

    NeighborsResponseMessage(
        BytesShared buffer)
        noexcept;


    const MessageType typeID() const
        noexcept;

    std::vector& neighbors() const
        noexcept;

    void appendNeighbor(
        const NodeUUID &nodeUUID)
        throw (bad_alloc, OverflowError);

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc &);

protected:
    std::vector<NodeUUID> mNeighbors;
};


#endif //GEO_NETWORK_CLIENT_NEIGHBORSRESPONSEMESSAGE_H
