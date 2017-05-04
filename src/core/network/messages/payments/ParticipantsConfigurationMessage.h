#ifndef GEO_NETWORK_CLIENT_PARTICIPANTSFINALCONFIGURATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_PARTICIPANTSFINALCONFIGURATIONMESSAGE_H


#include "../base/transaction/TransactionMessage.h"

#include "../../../common/multiprecision/MultiprecisionUtils.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../../common/exceptions/RuntimeError.h"


#include "boost/container/flat_set.hpp"
#include <tuple>

/**
 * This message is used by the intermediate nodes and receiver node
 * to be able to know what final paths configuration was built on the coordinator.
 *
 * It is very common for the payment operation,
 * to decrease common available payment amount due to each one node, discovered on the path.
 * As a result - it is possible, that nodes that was discovered earlier will reserve greater amount,
 * than nodes, that was later.
 *
 * But in final, all nodes of the each one path must commit operation with common for all involved nodes amount.
 * It is also very possible, that one node would be involved into several paths, that crosses this node.
 *
 * Only coordinator knows the final payment path.
 * This message is used to inform each one participant about it's final payment configuration.
 *
 *
 * todo:
 * WARN: nodes must enshure this message doesn't aks to increase reservation.
 */
class ParticipantsConfigurationMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<ParticipantsConfigurationMessage> Shared;
    typedef shared_ptr<const ParticipantsConfigurationMessage> ConstShared;
    typedef boost::container::flat_set<NodeUUID> NodesSet;


public:
    enum Designation {
        ForIntermediateNode = 0,
        ForReceiverNode = 1,
    };

public:
    ParticipantsConfigurationMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const Designation designation)
        noexcept;

    ParticipantsConfigurationMessage(
        BytesShared buffer)
        throw (bad_alloc);

    void addPath(
        const TrustLineAmount &commonPathAmount,
        const NodeUUID &incomingNode,
        const PathUUID &pathUUID)
        throw (ValueError, bad_alloc);

    void addPath(
        const TrustLineAmount &commonPathAmount,
        const NodeUUID &incomingNode,
        const NodeUUID &outgoingNode,
        const PathUUID &pathUUID)
        throw (ValueError, bad_alloc);

    const vector< tuple<NodesSet, ConstSharedTrustLineAmount, PathUUID>>& nodesAndFinalReservationAmount()
        const noexcept;

    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
    void addPath(
        const NodesSet &nodes,
        const TrustLineAmount &commonPathAmount,
        const PathUUID &pathUUID)
        throw (ValueError, bad_alloc);

protected:
    typedef uint32_t RecordsCount;

    // TODO: make it const
    pair<BytesShared, size_t> serializeForIntermediateNode() const
        throw (bad_alloc);

    // TODO: make it const
    pair<BytesShared, size_t> serializeForReceiverNode() const
        throw (bad_alloc);

    virtual void deserializeFromBytes(
        BytesShared buffer)
        throw (bad_alloc);

protected:
    // Specifies what kind of node should be receive this message.
    // In case if this message is addressed to the intermediate node - it's format would have one stucture,
    // and another one in case if this message is addressed to the receiver node.
    Designation mDesignation;

    // Specifies paths configuration for payment transaction.
    //
    // Scheme for intermediate nodes:
    // Paths = ({NodeUUID, NodeUUID, CommonPathAmount}, ..., {NodeUUID, NodeUUID, CommonPathAmount})
    //
    // Scheme for receiver node:
    // Paths = ({NodeUUID, CommonPathAmount}, ..., {NodeUUID, CommonPathAmount})
    // (second NodeUUID is omitted because receiver node doesn't have outgoing nodes involved into the transaction)
    vector< tuple<NodesSet, ConstSharedTrustLineAmount, PathUUID>> mPathsConfiguration;
};
#endif //GEO_NETWORK_CLIENT_PARTICIPANTSFINALCONFIGURATIONMESSAGE_H
