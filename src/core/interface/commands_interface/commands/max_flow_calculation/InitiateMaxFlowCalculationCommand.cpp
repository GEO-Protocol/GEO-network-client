#include "InitiateMaxFlowCalculationCommand.h"

InitiateMaxFlowCalculationCommand::InitiateMaxFlowCalculationCommand(
    const CommandUUID &uuid,
    const string &command):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 3;
    if (command.size() < minCommandLength) {
        throw ValueError(
                "InitiateMaxFlowCalculationCommand: can't parse command. "
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
                "InitiateMaxFlowCalculationCommand: can't parse command. "
                    "Error occurred while parsing  'count contractors' token.");
    }

    mContractors.reserve(mContractorsCount);
    size_t contractorStartPoint = tokenSeparatorPos + 1;
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
            throw ValueError(
                    "InitiateMaxFlowCalculationCommand: can't parse command. "
                        "Error occurred while parsing 'Contractor UUID' token.");
        }
    }

    mContractorAddresses.reserve(mContractorsCount);
    size_t contractorAddressStartPoint = contractorStartPoint;
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
                    "InitiateMaxFlowCalculationCommand: can't parse command. "
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

const string &InitiateMaxFlowCalculationCommand::identifier()
{
    static const string identifier = "GET:contractors/transactions/max";
    return identifier;
}

const vector<NodeUUID>& InitiateMaxFlowCalculationCommand::contractors() const
{
    return mContractors;
}

const vector<BaseAddress::Shared>& InitiateMaxFlowCalculationCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const SerializedEquivalent InitiateMaxFlowCalculationCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst InitiateMaxFlowCalculationCommand::responseOk(
    string &maxFlowAmount) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            maxFlowAmount));
}