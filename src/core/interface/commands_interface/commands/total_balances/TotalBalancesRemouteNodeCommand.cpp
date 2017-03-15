#include "TotalBalancesRemouteNodeCommand.h"

TotalBalancesRemouteNodeCommand::TotalBalancesRemouteNodeCommand(
        const CommandUUID &uuid,
        const string &commandBuffer):

        BaseUserCommand(
                uuid,
                identifier()) {

    parse(commandBuffer);
}

TotalBalancesRemouteNodeCommand::TotalBalancesRemouteNodeCommand(
        BytesShared buffer) :

        BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &TotalBalancesRemouteNodeCommand::identifier() {

    static const string identifier = "GET:nodes/stats/balances/total";
    return identifier;
}

const NodeUUID &TotalBalancesRemouteNodeCommand::contractorUUID() const {

    return mContractorUUID;
}

pair<BytesShared, size_t> TotalBalancesRemouteNodeCommand::serializeToBytes(){

    auto parentBytesAndCount = BaseUserCommand::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second +
                        NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
            dataBytesShared.get(),
            parentBytesAndCount.first.get(),
            parentBytesAndCount.second
    );
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    return make_pair(
            dataBytesShared,
            bytesCount);
}

void TotalBalancesRemouteNodeCommand::deserializeFromBytes(
        BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = BaseUserCommand::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
            mContractorUUID.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize);
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void TotalBalancesRemouteNodeCommand::parse(
        const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("TotalBalancesRemouteNodeCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
                0,
                NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("TotalBalancesRemouteNodeCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

CommandResult::SharedConst TotalBalancesRemouteNodeCommand::resultOk(string &totalBalancesStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            totalBalancesStr));
}

CommandResult::SharedConst TotalBalancesRemouteNodeCommand::resultNoResponse() const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            444));
}
