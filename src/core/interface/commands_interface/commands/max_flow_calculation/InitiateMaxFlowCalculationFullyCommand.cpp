#include "InitiateMaxFlowCalculationFullyCommand.h"

InitiateMaxFlowCalculationFullyCommand::InitiateMaxFlowCalculationFullyCommand(
    const CommandUUID &uuid,
    const string &command):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 3;
    if (command.size() < minCommandLength) {
        throw ValueError(
                "InitiateMaxFlowCalculationFullyCommand: can't parse command. "
                    "Received command is to short.");
    }

    size_t tokenSeparatorPos = command.find(kTokensSeparator);
    string contractorsCountStr = command.substr(
        0,
        tokenSeparatorPos);
    try {
        mContractorsCount = std::stoul(contractorsCountStr);
    } catch (...) {
        throw ValueError(
                "InitiateMaxFlowCalculationFullyCommand: can't parse command. "
                    "Error occurred while parsing  'count contractors' token.");
    }

    mContractorAddresses.reserve(mContractorsCount);
    size_t contractorAddressStartPoint = tokenSeparatorPos + 1;
    for (size_t idx = 0; idx < mContractorsCount; idx++) {
        try {
            tokenSeparatorPos = command.find(
                kTokensSeparator,
                contractorAddressStartPoint);
            string addressStr = command.substr(
                contractorAddressStartPoint,
                tokenSeparatorPos - contractorAddressStartPoint);
            mContractorAddresses.push_back(
                make_shared<IPv4WithPortAddress>(
                    addressStr));
            contractorAddressStartPoint = tokenSeparatorPos + 1;
        } catch (...) {
            throw ValueError(
                    "InitiateMaxFlowCalculationFullyCommand: can't parse command. "
                        "Error occurred while parsing 'Contractor Address' token.");
        }
    }

    size_t equivalentStartPoint = contractorAddressStartPoint;
    string equivalentStr = command.substr(
        equivalentStartPoint,
        command.size() - equivalentStartPoint - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "InitiateMaxFlowCalculationFullyCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &InitiateMaxFlowCalculationFullyCommand::identifier()
{
    static const string identifier = "GET:contractors/transactions/max/fully";
    return identifier;
}

const vector<BaseAddress::Shared>& InitiateMaxFlowCalculationFullyCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const SerializedEquivalent InitiateMaxFlowCalculationFullyCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst InitiateMaxFlowCalculationFullyCommand::responseOk(
    string &maxFlowAmount) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            maxFlowAmount));
}
