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


const CommandUUID &BaseUserCommand::UUID() const
{
    return mCommandUUID;
}

const string &BaseUserCommand::identifier() const
{
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
//    const_cast<mCommandUUID>;
    memcpy(
            const_cast<CommandUUID*>(&mCommandUUID),
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

CommandResult::SharedConst BaseUserCommand::responseOK() const
    noexcept
{
    return makeResult(200);
}

CommandResult::SharedConst BaseUserCommand::responseCreated() const
    noexcept
{
    return makeResult(201);
}

CommandResult::SharedConst BaseUserCommand::responsePostponedbyreservations() const
    noexcept
{
    return makeResult(203);
}

CommandResult::SharedConst BaseUserCommand::responseProtocolError() const
    noexcept
{
    return makeResult(401);
}

CommandResult::SharedConst BaseUserCommand::responseTrustlineIsAbsent() const
    noexcept
{
    return makeResult(405);
}

CommandResult::SharedConst BaseUserCommand::responseCurrentIncomingDebtIsGreaterThanNewAmount() const
    noexcept
{
    return makeResult(406);
}

CommandResult::SharedConst BaseUserCommand::responseTrustlineIsAlreadyPresent() const
    noexcept
{
    return makeResult(409);
}

CommandResult::SharedConst BaseUserCommand::responseTrustlineRejected() const
{
    return makeResult(402);
}

CommandResult::SharedConst BaseUserCommand::responseInsufficientFunds() const
    noexcept
{
    return makeResult(412);
}

CommandResult::SharedConst BaseUserCommand::responseConflictWithOtherOperation() const
    noexcept
{
    return makeResult(429);
}

CommandResult::SharedConst BaseUserCommand::responseRemoteNodeIsInaccessible() const
    noexcept
{
    return makeResult(444);
}

CommandResult::SharedConst BaseUserCommand::responseNoRoutes() const
    noexcept
{
    return makeResult(462);
}

CommandResult::SharedConst BaseUserCommand::responseUnexpectedError() const
    noexcept
{
    return makeResult(501);
}





