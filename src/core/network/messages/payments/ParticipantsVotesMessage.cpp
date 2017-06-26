#include "ParticipantsVotesMessage.h"


ParticipantsVotesMessage::ParticipantsVotesMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID,
    const NodeUUID &coordinatorUUID) :

    TransactionMessage(
        senderUUID,
        transactionUUID),
    mCoordinatorUUID(coordinatorUUID)
{}

ParticipantsVotesMessage::ParticipantsVotesMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    const auto kParticipantRecordSize =
        NodeUUID::kBytesSize
        + sizeof(SerializedVote);

    // Offsets
    const auto kCoordinatorUUIDOffset =
        buffer.get()
        + TransactionMessage::kOffsetToInheritedBytes();

    const auto kRecordsCountOffset =
        kCoordinatorUUIDOffset
        + NodeUUID::kBytesSize;

    const auto kFirstRecordOffset =
        kRecordsCountOffset
        + sizeof(RecordsCount);


    // Deserialization
    memcpy(
        mCoordinatorUUID.data,
        kCoordinatorUUIDOffset,
        NodeUUID::kBytesSize);

    auto currentOffset = kFirstRecordOffset;
    const RecordsCount kRecordsCount = *(kRecordsCountOffset);

    for (RecordsCount i=0; i<kRecordsCount; ++i) {
        NodeUUID participantUUID(currentOffset);

        const SerializedVote kVote =
            *(currentOffset + NodeUUID::kBytesSize);

        mVotes[participantUUID] = Vote(kVote);
        currentOffset += kParticipantRecordSize;
    }
}

ParticipantsVotesMessage::ParticipantsVotesMessage(
    const NodeUUID &senderUUID,
    const ParticipantsVotesMessage::Shared &message):
    TransactionMessage(
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
    if (mVotes.size() == numeric_limits<RecordsCount>::max()-1)
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
 * (if it's present i nthe votes list);
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
        + kCoordinatorUUIDSize
        + sizeof(RecordsCount)
        + kTotalParticipantsCount * kParticipantRecordSize;

    BytesShared buffer = tryMalloc(kBufferSize);

    // Offsets
    const auto kParentMessageOffset = buffer.get();
    const auto kCoordinatorUUIDOffset =
        kParentMessageOffset
        + parentBytesAndCount.second;

    const auto kParticipantsRecordsCountOffset =
        kCoordinatorUUIDOffset
        + kCoordinatorUUIDSize;

    const auto kFirstParticipantRecordOffset =
        kParticipantsRecordsCountOffset
        + sizeof(RecordsCount);

    // Parent message content
    memcpy(
        kParentMessageOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    // Coordinator UUID
    memcpy(
        kCoordinatorUUIDOffset,
        mCoordinatorUUID.data,
        kCoordinatorUUIDSize);

    // Records count
    memcpy(
        kParticipantsRecordsCountOffset,
        &kTotalParticipantsCount,
        sizeof(RecordsCount));

    // Nodes UUIDs and votes
    auto currentParticipantRecordOffset = kFirstParticipantRecordOffset;
    for (const auto NodeUUIDAndVote : mVotes) {

        const auto kParticipantUUID = NodeUUIDAndVote.first;
        memcpy(
            currentParticipantRecordOffset,
            &kParticipantUUID,
            NodeUUID::kBytesSize);

        const auto kVoteOffset = currentParticipantRecordOffset + NodeUUID::kBytesSize;
        const SerializedVote kVoteSerialized = NodeUUIDAndVote.second;
        memcpy(
            kVoteOffset,
            &kVoteSerialized,
            sizeof(kVoteSerialized));

        currentParticipantRecordOffset+=kParticipantRecordSize;
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
    if (participant == mCoordinatorUUID) {
        return;
    }
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
    for (const auto kNodeAndVote : mVotes) {
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

