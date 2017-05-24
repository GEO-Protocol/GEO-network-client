#include "InitiateMaxFlowCalculationCommand.h"

InitiateMaxFlowCalculationCommand::InitiateMaxFlowCalculationCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    parse(commandBuffer);
}

InitiateMaxFlowCalculationCommand::InitiateMaxFlowCalculationCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier())
{
    deserializeFromBytes(buffer);
}

const string &InitiateMaxFlowCalculationCommand::identifier()
{
    static const string identifier = "GET:contractors/transactions/max";
    return identifier;
}

const vector<NodeUUID>& InitiateMaxFlowCalculationCommand::contractors() const
{
    return mContractors;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void InitiateMaxFlowCalculationCommand::parse(
    const string &command)
{
    const auto amountTokenOffset = NodeUUID::kHexSize + 2;
    const auto minCommandLength = amountTokenOffset;
    if (command.size() < minCommandLength) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }
    size_t tabSeparator = command.find("\t");
    string contractorsCountStr = command.substr(
        0,
        tabSeparator);
    if (contractorsCountStr.at(0) == '-') {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                             "Can't parse command. 'count contractors' token can't be negative.");
    }
    try {
        mContractorsCount = std::stoul(contractorsCountStr);
    } catch (...) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                             "Can't parse command. Error occurred while parsing  'count contractors' token.");
    }
    mContractors.reserve(mContractorsCount);
    size_t contractorStartPoint = tabSeparator + 1;
    for (size_t idx = 0; idx < mContractorsCount; idx++) {
        try {
            string hexUUID = command.substr(
                contractorStartPoint,
                NodeUUID::kHexSize);
            mContractors.push_back(
                boost::lexical_cast<uuids::uuid>(
                    hexUUID));
            contractorStartPoint += NodeUUID::kHexSize + 1;
        } catch (...) {
            throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                                 "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
        }
    }
    if (contractorStartPoint + 1 < command.length()) {
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                             "Can't parse command. Disparity between command count contractors and real count contractors.");
    }
}

CommandResult::SharedConst InitiateMaxFlowCalculationCommand::responseOk(
    string &maxFlowAmount) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            UUID(),
            200,
            maxFlowAmount));
}