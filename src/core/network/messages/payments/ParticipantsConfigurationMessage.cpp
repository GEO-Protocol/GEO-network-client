#include "ParticipantsConfigurationMessage.h"


ParticipantsConfigurationMessage::ParticipantsConfigurationMessage(
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    const Designation designation)
    noexcept :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mDesignation(designation)
{}

ParticipantsConfigurationMessage::ParticipantsConfigurationMessage(
    BytesShared buffer)
    throw (bad_alloc) :

    TransactionMessage(buffer)
{
    deserializeFromBytes(buffer);
}

/**
 * Inserts new path specification into the message.
 *
 * @param commonPathAmount - amount that must be committed for the "incomingNode".
 * @param incomingNode - UUID of the neighbor for the addressee of this message.
 */
void ParticipantsConfigurationMessage::addPath (
    const TrustLineAmount &commonPathAmount,
    const NodeUUID &incomingNode,
    const PathUUID &pathUUID)
    throw (ValueError, bad_alloc)
{
    const NodesSet kNodesList = {incomingNode};
    addPath(
        kNodesList,
        commonPathAmount,
        pathUUID);
}

/**
 * Inserts new path specification into the message.
 *
 * @param commonPathAmount - amount that must be committed for the "incomingNode" and "outgoingNode".
 * @param incomingNode - UUID of the neighbor for the addressee of this message.
 */
void ParticipantsConfigurationMessage::addPath (
    const TrustLineAmount &commonPathAmount,
    const NodeUUID &incomingNode,
    const NodeUUID &outgoingNode,
    const PathUUID &pathUUID)
    throw (ValueError, bad_alloc)
{
    const NodesSet kNodesList = {incomingNode, outgoingNode};
    addPath(
        kNodesList,
        commonPathAmount,
        pathUUID);
}

/**
 * Inserts new path specification into the message.
 *
 * @param nodes - set of at max 2 node UUIDs, that are neighbor nodes for the addressee.
 * @param commonPathAmount - amount that must be committed for the "nodes".
 */
void ParticipantsConfigurationMessage::addPath (
    const NodesSet &nodes,
    const TrustLineAmount &commonPathAmount,
    const PathUUID &pathUUID)
    throw (ValueError, bad_alloc)
{
#ifdef INTERNAL_ARGUMENTS_VALIDATION
    if (0 == commonPathAmount)
        throw ValueError(
            "ParticipantsConfigurationMessage::addPath: "
            "common path amount can't be 0.");

    if (0 == nodes.size())
        throw ValueError(
            "ParticipantsConfigurationMessage::addPath: "
            "nodes set can't be empty.");

    if (nodes.size() > 2)
        throw ValueError(
            "ParticipantsConfigurationMessage::addPath: "
            "nodes set can't be greater than 2.");

    if (nodes.size() == 2 && *(nodes.cbegin()) == *(std::next(nodes.cbegin())))
        throw ValueError(
            "ParticipantsConfigurationMessage::addPath: "
            "nodes set contains equal nodes.");

    for (const auto &kRecords : mPathsConfiguration)
        if (std::get<2>(kRecords) == pathUUID)
            throw ValueError(
                "ParticipantsConfigurationMessage::addPath: "
                "message already contains this path.");
#endif

    const auto kRecord = std::make_tuple(
        nodes,
        make_shared<const TrustLineAmount>(
            commonPathAmount),
        pathUUID);

    mPathsConfiguration.push_back(kRecord);
}

const Message::MessageType ParticipantsConfigurationMessage::typeID () const
{
    return Message::Payments_ParticipantsPathsConfiguration;
}

pair<BytesShared, size_t> ParticipantsConfigurationMessage::serializeToBytes() const
    throw(bad_alloc)
{
    switch (mDesignation) {
        case Designation::ForIntermediateNode:
            return serializeForIntermediateNode();

        case Designation::ForReceiverNode:
            return serializeForReceiverNode();

        // Please, don't throw any other exception here.
        // Even in case if switch encounters default state,
        // that is practically impossible.
    }
}

/**
 * Serializes the message into shared bytes sequence.
 *
 * Message format:
 *  <parent message content>
 *
 * 1B - Path designation. See class specification for details.
 * 4B - Path records count.
 *
 *  { Path record
 *      16B - Incoming neighbor UUID;
 *      16B - Outgoing neighbor UUID;
 *      32B - Common path amount.
 *  }
 *
 * ...
 *
 * { Path record
 *      16B - Incoming neighbor UUID;
 *      16B - Outgoing neighbor UUID;
 *      32B - Common path amount.
 *  }
 *
 *
 *  @throws bad_alloc in case of insufficient memory.
 */
pair<BytesShared, size_t> ParticipantsConfigurationMessage::serializeForIntermediateNode() const
    throw (bad_alloc)
{
    const auto kParentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kTotalPathsCount = mPathsConfiguration.size();
    const auto kPathRecordSize =
        NodeUUID::kBytesSize * 2
        + kTrustLineAmountBytesCount
        + sizeof(PathUUID);

    const auto kBufferSize =
        kParentBytesAndCount.second
        + sizeof(byte)
        + sizeof(RecordsCount)
        + kTotalPathsCount * kPathRecordSize;

    BytesShared buffer = tryMalloc(kBufferSize);

    // Offsets
    const auto kParentMessageOffset = buffer.get();
    const auto kMessageDesignationOffset =
        kParentMessageOffset
        + kParentBytesAndCount.second;

    const auto kPathsRecordsCountOffset =
        kMessageDesignationOffset
        + sizeof(byte);

    const auto kFirstPathRecordOffset =
        kPathsRecordsCountOffset
        + sizeof(RecordsCount);

    // Parent message content
    memcpy(
        kParentMessageOffset,
        kParentBytesAndCount.first.get(),
        kParentBytesAndCount.second);

    // Message designation
    const byte kSerializedDesignation = (byte)mDesignation;
    memcpy(
        kMessageDesignationOffset,
        &kSerializedDesignation,
        sizeof(byte));

    // Records count
    memcpy(
        kPathsRecordsCountOffset,
        &kTotalPathsCount,
        sizeof(RecordsCount));

    // Nodes UUIDs and amount
    auto currentPathRecordOffset = kFirstPathRecordOffset;
    for (const auto kNodesAndAmountAndPathUUID : mPathsConfiguration) {

        const auto kIncomingNeighborUUID = *(std::get<0>(kNodesAndAmountAndPathUUID).cbegin());
        const auto kOutgoingNeighborUUID = *(std::get<0>(kNodesAndAmountAndPathUUID).cbegin());

        const auto kAmount = *(std::get<1>(kNodesAndAmountAndPathUUID));
        const auto kSerializedAmount = trustLineAmountToBytes(kAmount);

        const auto kSerializedPathUUID = std::get<2>(kNodesAndAmountAndPathUUID);

        memcpy(
            currentPathRecordOffset,
            &kIncomingNeighborUUID,
            NodeUUID::kBytesSize);

        memcpy(
            currentPathRecordOffset + NodeUUID::kBytesSize,
            &kOutgoingNeighborUUID,
            NodeUUID::kBytesSize);

        memcpy(
            currentPathRecordOffset + NodeUUID::kBytesSize * 2,
            kSerializedAmount.data(),
            kSerializedAmount.size());

        memcpy(
            currentPathRecordOffset + NodeUUID::kBytesSize * 2 + kSerializedAmount.size(),
            &kSerializedPathUUID,
            sizeof(PathUUID));

        currentPathRecordOffset += kPathRecordSize;
    }

    return make_pair(
        buffer,
        kBufferSize);
}

/**
 * Serializes the message into shared bytes sequence.
 *
 * Message format:
 *  <parent message content>
 *
 * 1B - Path designation. See class specification for details.
 * 4B - Path records count;
 *
 *  { Path record
 *      16B - Incoming neighbor UUID;
 *      32B - Common path amount.
 *  }
 *
 * ...
 *
 * { Path record
 *      16B - Incoming neighbor UUID;
 *      32B - Common path amount.
 *  }
 *
 *
 *  @throws bad_alloc in case of insufficient memory.
 */
pair<BytesShared, size_t> ParticipantsConfigurationMessage::serializeForReceiverNode() const
    throw (bad_alloc)
{
    const auto kParentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kTotalPathsCount = mPathsConfiguration.size();
    const auto kPathRecordSize =
        NodeUUID::kBytesSize
        + kTrustLineAmountBytesCount
        + sizeof(PathUUID);

    const auto kBufferSize =
        kParentBytesAndCount.second
        + sizeof(byte)
        + sizeof(RecordsCount)
        + kTotalPathsCount * kPathRecordSize;

    BytesShared buffer = tryMalloc(kBufferSize);

    // Offsets
    const auto kParentMessageOffset = buffer.get();
    const auto kMessageDesignationOffset =
        kParentMessageOffset
        + kParentBytesAndCount.second;

    const auto kPathsRecordsCountOffset =
        kMessageDesignationOffset
        + sizeof(byte);

    const auto kFirstPathRecordOffset =
        kPathsRecordsCountOffset
        + sizeof(RecordsCount);

    // Parent message content
    memcpy(
        kParentMessageOffset,
        kParentBytesAndCount.first.get(),
        kParentBytesAndCount.second);

    // Message designation
    const byte kSerializedDesignation = (byte)mDesignation;
    memcpy(
        kMessageDesignationOffset,
        &kSerializedDesignation,
        sizeof(byte));

    // Records count
    memcpy(
        kPathsRecordsCountOffset,
        &kTotalPathsCount,
        sizeof(RecordsCount));

    // Nodes UUIDs and amount
    auto currentPathRecordOffset = kFirstPathRecordOffset;
    for (const auto kNodesAndAmountAndPathUUID : mPathsConfiguration) {

        const auto kIncomingNeighborUUID = *(std::get<0>(kNodesAndAmountAndPathUUID).cbegin());
        const auto kAmount = *(std::get<1>(kNodesAndAmountAndPathUUID));
        const auto kSerializedAmount = trustLineAmountToBytes(kAmount);
        const auto kSerializedPathUUID = std::get<2>(kNodesAndAmountAndPathUUID);

        memcpy(
            currentPathRecordOffset,
            &kIncomingNeighborUUID,
            NodeUUID::kBytesSize);

        memcpy(
            currentPathRecordOffset + NodeUUID::kBytesSize,
            kSerializedAmount.data(),
            kSerializedAmount.size());

        memcpy(
            currentPathRecordOffset + NodeUUID::kBytesSize + kSerializedAmount.size(),
            &kSerializedPathUUID,
            sizeof(PathUUID));

        currentPathRecordOffset += kPathRecordSize;
    }

    return make_pair(
        buffer,
        kBufferSize);
}

void ParticipantsConfigurationMessage::deserializeFromBytes (
    BytesShared buffer)
    throw (bad_alloc)
{
    // Offsets
    const auto kDesignationOffset =
        buffer.get()
        + TransactionMessage::kOffsetToInheritedBytes();

    const auto kRecordsCountOffset =
        kDesignationOffset
        + sizeof(byte);

    const auto kFirstRecordOffset =
        kRecordsCountOffset
        + sizeof(RecordsCount);


    // Deserialization
    byte serializedDesignation;
    memcpy(
        &serializedDesignation,
        kDesignationOffset,
        sizeof(byte));

    RecordsCount recordsCount;
    memcpy(
        &recordsCount,
        kRecordsCountOffset,
        sizeof(RecordsCount));


    auto currentOffset = kFirstRecordOffset;
    for (RecordsCount i=0; i<recordsCount; ++i) {

        if (Designation(serializedDesignation) == Designation::ForReceiverNode) {
            NodeUUID incomingNodeUUID;
            memcpy(
                incomingNodeUUID.data,
                currentOffset,
                NodeUUID::kBytesSize);

            vector<byte> commonPathAmountBytes(kTrustLineBalanceBytesCount);
            memcpy(
                commonPathAmountBytes.data(),
                currentOffset + NodeUUID::kBytesSize,
                kTrustLineAmountBytesCount);

            PathUUID *pathUUID = new (currentOffset + NodeUUID::kBytesSize + kTrustLineAmountBytesCount) PathUUID;

            const TrustLineAmount kCommonPathAmount = bytesToTrustLineAmount(commonPathAmountBytes);
            const NodesSet kNodesSet = {incomingNodeUUID};
            const PathUUID kPathUUID = *pathUUID;
            mPathsConfiguration.push_back(
                std::make_tuple(
                    kNodesSet,
                    make_shared<const TrustLineAmount>(
                        kCommonPathAmount),
                    kPathUUID));

            continue;
        }

        if (Designation(serializedDesignation) == Designation::ForIntermediateNode) {
            NodeUUID incomingNodeUUID;
            memcpy(
                incomingNodeUUID.data,
                currentOffset,
                NodeUUID::kBytesSize);

            NodeUUID outgoingNodeUUID;
            memcpy(
                outgoingNodeUUID.data,
                currentOffset + NodeUUID::kBytesSize,
                NodeUUID::kBytesSize);

            vector<byte> commonPathAmountBytes(kTrustLineBalanceBytesCount);
            memcpy(
                commonPathAmountBytes.data(),
                currentOffset + NodeUUID::kBytesSize * 2,
                kTrustLineAmountBytesCount);

            PathUUID *pathUUID = new (currentOffset + NodeUUID::kBytesSize * 2 + kTrustLineAmountBytesCount) PathUUID;

            const TrustLineAmount kCommonPathAmount = bytesToTrustLineAmount(commonPathAmountBytes);
            const NodesSet kNodesSet = {incomingNodeUUID, outgoingNodeUUID};
            const PathUUID kPathUUID = *pathUUID;
            mPathsConfiguration.push_back(
                std::make_tuple(
                    kNodesSet,
                    make_shared<const TrustLineAmount>(
                        kCommonPathAmount),
                    kPathUUID));

            continue;
        }
    }
}

const vector< tuple<ParticipantsConfigurationMessage::NodesSet, ConstSharedTrustLineAmount, Message::PathUUID>>&
ParticipantsConfigurationMessage::nodesAndFinalReservationAmount () const
    noexcept
{
    return mPathsConfiguration;
}
