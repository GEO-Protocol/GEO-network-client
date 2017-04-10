#include "BaseUserCommand.h"

BaseUserCommand::BaseUserCommand(
    const string& identifier) :

    mCommandIdentifier(identifier){}

BaseUserCommand::BaseUserCommand(
    const CommandUUID &commandUUID,
    const string &identifier) :

    mCommandUUID(commandUUID),
    mCommandIdentifier(identifier),
    mTimestampAccepted(utc_now()) {}


const CommandUUID &BaseUserCommand::UUID() const {

    return mCommandUUID;
}

const string &BaseUserCommand::identifier() const {

    return mCommandIdentifier;
}

const DateTime &BaseUserCommand::timestampAccepted() const {

    return mTimestampAccepted;
}

pair<BytesShared, size_t> BaseUserCommand::serializeToBytes() {

    size_t bytesCount = CommandUUID::kBytesSize + sizeof(GEOEpochTimestamp);
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //-----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        mCommandUUID.data,
        CommandUUID::kBytesSize
    );
    dataBytesOffset += CommandUUID::kBytesSize;
    //-----------------------------------------------------
    GEOEpochTimestamp timestamp = microsecondsSinceGEOEpoch(mTimestampAccepted);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &timestamp,
        sizeof(GEOEpochTimestamp)
    );
    //-----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount
    );
}

void BaseUserCommand::deserializeFromBytes(
    BytesShared buffer) {

    size_t bytesBufferOffset = 0;
    //-----------------------------------------------------
    memcpy(
        mCommandUUID.data,
        buffer.get(),
        CommandUUID::kBytesSize
    );
    bytesBufferOffset += CommandUUID::kBytesSize;
    //-----------------------------------------------------
    uint64_t *commandAcceptedTimestamp = new (buffer.get() + bytesBufferOffset) uint64_t;
    mTimestampAccepted = dateTimeFromGEOEpochTimestamp((GEOEpochTimestamp) *commandAcceptedTimestamp);
}

const size_t BaseUserCommand::kOffsetToInheritedBytes() {

    static const size_t offset = CommandUUID::kHexSize + sizeof(GEOEpochTimestamp);
    return offset;
}

CommandResult::SharedConst BaseUserCommand::unexpectedErrorResult() {
    return CommandResult::Shared(
        new CommandResult(
            mCommandUUID,
            501
        )
    );
}

/**
 * Shortcut for creating results in derived commands classes.
 *
 * @param code - result code that would be transferred out of the engine.
 */
CommandResult::SharedConst BaseUserCommand::makeResult(
    const uint16_t code) const
{
    return make_shared<const CommandResult>(UUID(), code);
}
