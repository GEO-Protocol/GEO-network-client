#include "TotalBalancesCommand.h"

TotalBalancesCommand::TotalBalancesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer):

        BaseUserCommand(
                uuid,
                identifier()) {

    parse(commandBuffer);
}

TotalBalancesCommand::TotalBalancesCommand(
        BytesShared buffer) :

        BaseUserCommand(identifier()) {

    deserializeFromBytes(buffer);
}

const string &TotalBalancesCommand::identifier() {

    static const string identifier = "GET:/stats/balances/total";
    return identifier;
}

pair<BytesShared, size_t> TotalBalancesCommand::serializeToBytes(){

    //auto parentBytesAndCount = BaseUserCommand::serializeToBytes();

    return BaseUserCommand::serializeToBytes();

    /*size_t bytesCount = parentBytesAndCount.second +
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
            bytesCount);*/
}

void TotalBalancesCommand::deserializeFromBytes(
        BytesShared buffer) {

    BaseUserCommand::deserializeFromBytes(buffer);
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void TotalBalancesCommand::parse(
        const string &command) {

    /*const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset;

    if (command.size() < minCommandLength) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                                 "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
                0,
                NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("TotalBalancesCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }*/
}

CommandResult::SharedConst TotalBalancesCommand::resultOk(string &totalBalancesStr) const {

    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            totalBalancesStr));
}
