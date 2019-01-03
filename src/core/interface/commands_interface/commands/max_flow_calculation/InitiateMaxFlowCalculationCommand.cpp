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

    mContractorAddresses.reserve(mContractorsCount);
    size_t contractorAddressStartPoint = tokenSeparatorPos + 1;
    for (size_t idx = 0; idx < mContractorsCount; idx++) {
        try {
            tokenSeparatorPos = command.find(
                kTokensSeparator,
                contractorAddressStartPoint);

            string addressWithTypeStr = command.substr(
                contractorAddressStartPoint,
                tokenSeparatorPos - contractorAddressStartPoint);
            auto addressTypeSeparatorPos = addressWithTypeStr.find(kAddressTypeSeparator);
            auto addressTypeStr = addressWithTypeStr.substr(0, addressTypeSeparatorPos);
            auto addressType = (BaseAddress::AddressType)std::stoul(addressTypeStr);
            auto addressStr = addressWithTypeStr.substr(
                addressTypeSeparatorPos + 1,
                addressWithTypeStr.length() - addressTypeSeparatorPos + 1);
            switch (addressType) {
                case BaseAddress::IPv4_IncludingPort: {
                    mContractorAddresses.push_back(
                        make_shared<IPv4WithPortAddress>(
                            addressStr));
                    break;
                }
                default:
                    throw ValueError(
                            "InitiateMaxFlowCalculationCommand: can't parse command. "
                                "Error occurred while parsing 'Contractor Address' token.");

            }
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