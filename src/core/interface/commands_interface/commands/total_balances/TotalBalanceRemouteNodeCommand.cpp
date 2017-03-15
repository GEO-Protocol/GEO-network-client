#include "TotalBalanceRemouteNodeCommand.h"

TotalBalanceRemouteNodeCommand::TotalBalanceRemouteNodeCommand(
        const CommandUUID &uuid,
        const string &commandBuffer):

        BaseUserCommand(
                uuid,
                identifier()) {

    parse(commandBuffer);
}

TotalBalanceRemouteNodeCommand::TotalBalanceRemouteNodeCommand(
        BytesShared buffer) :

        BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &TotalBalanceRemouteNodeCommand::identifier() {

    static const string identifier = "GET:nodes/stats/balances/total/";
    return identifier;
}

const NodeUUID &TotalBalanceRemouteNodeCommand::contractorUUID() const {

    return mContractorUUID;
}

pair<BytesShared, size_t> TotalBalanceRemouteNodeCommand::serializeToBytes(){

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

void TotalBalanceRemouteNodeCommand::deserializeFromBytes(
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
void TotalBalanceRemouteNodeCommand::parse(
        const string &command) {

    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("TotalBalanceRemouteNodeCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
                0,
                NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("TotalBalanceRemouteNodeCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

CommandResult::SharedConst TotalBalanceRemouteNodeCommand::resultOk(string &totalBalancesStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            totalBalancesStr));
}
