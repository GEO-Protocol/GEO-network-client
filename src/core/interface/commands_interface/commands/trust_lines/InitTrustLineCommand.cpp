#include "InitTrustLineCommand.h"

InitTrustLineCommand::InitTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = 7;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "InitTrustLineCommand: can't parse command. "
                "Received command is to short.");
    }

    size_t tokenSeparatorPos = command.find(kTokensSeparator);
    auto contractorAddressesCntStr = command.substr(0, tokenSeparatorPos);
    try {
        mContractorAddressesCount = (uint32_t)std::stoul(contractorAddressesCntStr);
    } catch (...) {
        throw ValueError(
            "InitTrustLineCommand: can't parse command. "
                "Error occurred while parsing  'contractor addresses count' token.");
    }

    mContractorAddresses.reserve(mContractorAddressesCount);
    size_t contractorAddressStartPos = tokenSeparatorPos + 1;
    for (size_t idx = 0; idx < mContractorAddressesCount; idx++) {
        try {
            tokenSeparatorPos = command.find(
                kTokensSeparator,
                contractorAddressStartPos);
            string addressWithTypeStr = command.substr(
                contractorAddressStartPos,
                tokenSeparatorPos - contractorAddressStartPos);
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
                        "InitTrustLineCommand: can't parse command. "
                            "Error occurred while parsing 'Contractor Address' token.");
            }
            contractorAddressStartPos = tokenSeparatorPos + 1;
        } catch (...) {
            throw ValueError(
                "InitTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
        }
    }

    size_t equivalentOffset = tokenSeparatorPos + 1;
    string equivalentStr = command.substr(
        equivalentOffset,
        command.size() - equivalentOffset - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
            "InitTrustLineCommand: can't parse command. "
                "Error occurred while parsing  'equivalent' token.");
    }
}

const string &InitTrustLineCommand::identifier()
noexcept
{
    static const string identifier = "INIT:contractors/trust-line";
    return identifier;
}

const SerializedEquivalent InitTrustLineCommand::equivalent() const
noexcept
{
    return mEquivalent;
}

vector<BaseAddress::Shared> InitTrustLineCommand::contractorAddresses() const
noexcept
{
    return mContractorAddresses;
}