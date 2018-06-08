#include "ParticipantsVotesMessage.h"


ParticipantsVotesMessage::ParticipantsVotesMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const NodeUUID &coordinatorUUID) :

    TransactionMessage(
        equivalent,
        senderUUID,
        transactionUUID),
    mCoordinatorUUID(coordinatorUUID)
{}

ParticipantsVotesMessage::ParticipantsVotesMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    auto bytesBufferOffset = TransactionMessage::kOffsetToInheritedBytes();
    size_t kParticipantRecordSize =
        NodeUUID::kBytesSize
        + sizeof(SerializedVote);

    // Deserialization
    memcpy(
        mCoordinatorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize);
    bytesBufferOffset += NodeUUID::kBytesSize;

    SerializedRecordsCount kRecordsCount;
    memcpy(
        &kRecordsCount,
        buffer.get() + bytesBufferOffset,
        sizeof(SerializedRecordsCount));
    bytesBufferOffset += sizeof(SerializedRecordsCount);

    for (SerializedRecordNumber i=0; i<kRecordsCount; ++i) {
        NodeUUID participantUUID(buffer.get() + bytesBufferOffset);

        const SerializedVote kVote =
            *(buffer.get() + bytesBufferOffset + NodeUUID::kBytesSize);

        mVotes[participantUUID] = Vote(kVote);
        bytesBufferOffset += kParticipantRecordSize;
    }
}

ParticipantsVotesMessage::ParticipantsVotesMessage(
    const NodeUUID &senderUUID,
    const ParticipantsVotesMessage::Shared &message):
    TransactionMessage(
        message->equivalent(),
        senderUUID,
        message->transactionUUID()),
    mCoordinatorUUID(message->coordinatorUUID()),
    mVotes(message->votes())
{}

/**
 * Inserts new participant into participants list with default vote (Uncertain).
 *
 * @throws OverflowError in case if no more participants can be added.
 */
void ParticipantsVotesMessage::addParticipant(
    const NodeUUID &participant)
{
    if (mVotes.size() == numeric_limits<SerializedRecordsCount>::max()-1)
        throw OverflowError(
            "ParticipantsVotesMessage::addParticipant: "
            "no more new participants can be added.");

    mVotes[participant] = Uncertain;
}

/**
 * Returns UUID of the node, that, by the protocol, must receive this message
 * in case if current node approves the operation.
 *
 * @throws NotFoundError in case if current node is last in votes list, or is absent.
 */
const NodeUUID& ParticipantsVotesMessage::nextParticipant(
    const NodeUUID& currentNodeUUID) const
{
    auto kNodeUUIDAndVote = mVotes.find(currentNodeUUID);
    if (next(kNodeUUIDAndVote) == mVotes.end())
        throw NotFoundError(
            "ParticipantsApprovingMessage::nextParticipant: "
            "there are no nodes left in the votes list.");

    return (next(kNodeUUIDAndVote))->first;
}

const NodeUUID &ParticipantsVotesMessage::coordinatorUUID () const
{
    return mCoordinatorUUID;
}

/**
 * Returns UUID of the node, that, by the protocol,
 * must receive this message right after the coordinator.
 *
 * @throws NotFoundError in case if no nodes are present in the votes list yet.
 */
const NodeUUID& ParticipantsVotesMessage::firstParticipant() const
{
    if (mVotes.empty())
        throw NotFoundError(
            "ParticipantsApprovingMessage::firstParticipant: "
            "there are no nodes in votes list yet.");

    return mVotes.cbegin()->first;
}

/**
 * Returns vote of the "participant"
 * (if it's present in the votes list);
 *
 * @throws NotFoundError in case if no vote is present for "participant";
 */
ParticipantsVotesMessage::Vote ParticipantsVotesMessage::vote(
    const NodeUUID &participant) const
{
    if (mVotes.count(participant) == 0)
        throw NotFoundError(
            "ParticipantsVotesMessage::vote: "
            "there is no such participant in the votes list.");

    return mVotes.at(participant);
}

const Message::MessageType ParticipantsVotesMessage::typeID() const
{
    return Message::Payments_ParticipantsVotes;
}

/**
 * Serializes the message into shared bytes sequence.
 *
 * Message format:
 *  16B - Transaction UUID,
 *  16B - Coordinator UUID,
 *  4B  - Total participants count,
 *
 *  { Participant record
 *      16B - Participant 1 UUID,
 *      1B  - Participant 1 vote (true/false),
 *  }
 *
 *  ...
 *
 *  { Participant record
 *      16B - Participant N UUID
 *      1B  - Participant N vote (true/false)
 *  }
 *
 *
 *  @throws bad_alloc in case of insufficient memory.
 */
pair<BytesShared, size_t> ParticipantsVotesMessage::serializeToBytes() const
    throw(bad_alloc)
{
    const auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kTotalParticipantsCount = mVotes.size();
    const auto kCoordinatorUUIDSize = NodeUUID::kBytesSize;
    const auto kParticipantRecordSize =
        NodeUUID::kBytesSize
        + sizeof(SerializedVote);

    const auto kBufferSize =
        parentBytesAndCount.second
        + NodeUUID::kBytesSize
        + sizeof(SerializedRecordsCount)
        + kTotalParticipantsCount * kParticipantRecordSize;

    BytesShared buffer = tryMalloc(kBufferSize);

    size_t dataBytesOffset = 0;
    // Parent message content
    memcpy(
        buffer.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;

    // Coordinator UUID
    memcpy(
        buffer.get() + dataBytesOffset,
        mCoordinatorUUID.data,
        kCoordinatorUUIDSize);
    dataBytesOffset += NodeUUID::kBytesSize;

    // Records count
    memcpy(
        buffer.get() + dataBytesOffset,
        &kTotalParticipantsCount,
        sizeof(SerializedRecordsCount));
    dataBytesOffset += sizeof(SerializedRecordsCount);

    // Nodes UUIDs and votes
    for (const auto &nodeUUIDAndVote : mVotes) {

        const auto kParticipantUUID = nodeUUIDAndVote.first;
        memcpy(
            buffer.get() + dataBytesOffset,
            kParticipantUUID.data,
            NodeUUID::kBytesSize);
        dataBytesOffset += NodeUUID::kBytesSize;

        const SerializedVote kVoteSerialized = nodeUUIDAndVote.second;
        memcpy(
            buffer.get() + dataBytesOffset,
            &kVoteSerialized,
            sizeof(kVoteSerialized));
        dataBytesOffset += sizeof(kVoteSerialized);
    }

    return make_pair(
        buffer,
        kBufferSize);
}

/**
 * Sets vote of the "participant" to "rejected".
 * Checks if "participant" is listed in votes list.
 *
 * @throws NotFoundError - in case if received "participant" doesn't listed in votes list.
 */
void ParticipantsVotesMessage::reject(
    const NodeUUID &participant)
{
    if (mVotes.count(participant) != 1)
        throw NotFoundError(
                "ParticipantsApprovingMessage::reject: "
                "received participant doesn't listed in votes list.");

    mVotes[participant] = Vote::Rejected;
}

/**
 * Sets vote of the "participant" to "approved".
 * Checks if "participant" is listed in votes list.
 *
 * @throws NotFoundError - in case if received "participant" doesn't listed in votes list.
 */
void ParticipantsVotesMessage::approve(
    const NodeUUID &participant)
{
    if (mVotes.count(participant) != 1)
        throw NotFoundError(
            "ParticipantsApprovingMessage::approve: "
            "received participant doesn't listed in votes list.");

    mVotes[participant] = Vote::Approved;
}

bool ParticipantsVotesMessage::containsRejectVote() const {
    for (const auto &participantAndVote : mVotes) {
        if (participantAndVote.second == Vote::Rejected)
            return true;
    }

    return false;
}

/**
 * Returns true in case if all participants voted for transaction approving.
 * Otherwise - returns false;
 */
bool ParticipantsVotesMessage::achievedConsensus() const {
    for (const auto &kNodeAndVote : mVotes) {
        if (kNodeAndVote.second == Vote::Uncertain or
            kNodeAndVote.second == Vote::Rejected)
            return false;
    }

    return true;
}

size_t ParticipantsVotesMessage::participantsCount () const
{
    return mVotes.size();
}

const boost::container::flat_map<NodeUUID, ParticipantsVotesMessage::Vote>& ParticipantsVotesMessage::votes() const
{
    return mVotes;
}

bool ParticipantsVotesMessage::containsParticipant(
    const NodeUUID &node) const
{
    return mVotes.find(node) != mVotes.end();
}

