#include "BaseUserCommand.h"

BaseUserCommand::BaseUserCommand(
    const string& identifier) :

    mCommandIdentifier(identifier){}

BaseUserCommand::BaseUserCommand(
    const CommandUUID &commandUUID,
    const string &identifier) :

    mCommandUUID(commandUUID),
    mCommandIdentifier(identifier),
    mTimestampAccepted(posix::microsec_clock::universal_time()) {}


const CommandUUID &BaseUserCommand::UUID() const {

    return mCommandUUID;
}

const string &BaseUserCommand::identifier() const {

    return mCommandIdentifier;
}

const Timestamp &BaseUserCommand::timestampAccepted() const {

    return mTimestampAccepted;
}

pair<BytesShared, size_t> BaseUserCommand::serializeToBytes() const {

    size_t bytesCount = CommandUUID::kBytesSize + sizeof(MicrosecondsTimestamp);
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
    MicrosecondsTimestamp timestamp = microsecondsTimestamp(mTimestampAccepted);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &timestamp,
        sizeof(MicrosecondsTimestamp)
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
    mTimestampAccepted = posixTimestamp((MicrosecondsTimestamp) *commandAcceptedTimestamp);
}

const size_t BaseUserCommand::kOffsetToInheritedBytes() {

    static const size_t offset = CommandUUID::kHexSize + sizeof(MicrosecondsTimestamp);
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