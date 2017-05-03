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

const NodeUUID &InitiateMaxFlowCalculationCommand::contractorUUID() const
{
    return mContractorUUID;
}

/**
 * Throws ValueError if deserialization was unsuccessful.
 */
void InitiateMaxFlowCalculationCommand::parse(
    const string &command)
{
    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
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
        throw ValueError("InitiateMaxFlowCalculationCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
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