#include "BaseUserCommand.h"

BaseUserCommand::BaseUserCommand() {}

BaseUserCommand::BaseUserCommand(
    const CommandUUID &commandUUID,
    const string &identifier) :

    mCommandUUID(commandUUID),
    mCommandIdentifier(identifier),
    mTimestampAccepted(posix::microsec_clock::universal_time()) {}


const CommandUUID &BaseUserCommand::commandUUID() const {
    return mCommandUUID;
}

const string &BaseUserCommand::commandIdentifier() const {
    return mCommandIdentifier;
}

const Timestamp &BaseUserCommand::timestampAccepted() const {
    return mTimestampAccepted;
}

pair<BytesShared, size_t> BaseUserCommand::serializeParentToBytes() {

    size_t bytesCount = CommandUUID::kBytesSize + sizeof(MicrosecondsTimestamp);
    byte *data = (byte *) calloc(bytesCount, sizeof(byte));
    //-----------------------------------------------------
    memcpy(
        data,
        mCommandUUID.data,
        CommandUUID::kBytesSize
    );
    //-----------------------------------------------------
    Duration commandAcceptedDuration = mTimestampAccepted - kEpoch();
    MicrosecondsTimestamp commandAcceptedMicroseconds = (MicrosecondsTimestamp) commandAcceptedDuration.total_microseconds();
    memcpy(
        data + CommandUUID::kBytesSize,
        &commandAcceptedMicroseconds,
        sizeof(MicrosecondsTimestamp)
    );
    //-----------------------------------------------------
    return make_pair(
        BytesShared(data, free),
        bytesCount
    );
}

void BaseUserCommand::deserializeParentFromBytes(
    BytesShared buffer) {

    //-----------------------------------------------------
    memcpy(
        mCommandUUID.data,
        buffer.get(),
        CommandUUID::kBytesSize
    );
    //-----------------------------------------------------
    uint64_t *commandAcceptedTimestamp = new (buffer.get() + CommandUUID::kBytesSize) uint64_t;
    uint32_t maxUINT32_T = std::numeric_limits<uint32_t >::max();
    Timestamp accumulator = kEpoch();
    while (*commandAcceptedTimestamp > maxUINT32_T) {
        accumulator += posix::seconds(maxUINT32_T);
        *commandAcceptedTimestamp -= maxUINT32_T;
    }
    accumulator += posix::microseconds(*commandAcceptedTimestamp);
    mTimestampAccepted = accumulator;
}

// todo: (DM) change this to the GEO epoch (see TransactionState)
//
// todo: (DM) BaseUserCommand should not be timestamp poin in the system.
// todo: (DM) it's not a BaseUserCommand responsibility.
const Timestamp BaseUserCommand::kEpoch() {
    static const Timestamp epoch(boost::gregorian::date(1970, 1, 1));
    return epoch;
}

// todo: (DM) please, rename. it's not so obvious as it may be.
const size_t BaseUserCommand::kOffsetToInheritBytes() {
    static const size_t offset = CommandUUID::kHexSize + sizeof(MicrosecondsTimestamp);
    return offset;
}

const CommandResult* BaseUserCommand::unexpectedErrorResult() {
    CommandResult *result = new CommandResult(mCommandUUID, 501);
    return result;
}
