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

    size_t contractorAddressStartPos = 0;
    size_t tokenSeparatorPos = command.find(
        kTokensSeparator,
        contractorAddressStartPos);
    auto contractorAddressStr = command.substr(
        contractorAddressStartPos,
        tokenSeparatorPos - contractorAddressStartPos);
    try {
        mContractorAddress = make_shared<IPv4WithPortAddress>(
            contractorAddressStr);
    } catch (...) {
        throw ValueError(
            "InitTrustLineCommand: can't parse command. "
                "Error occurred while parsing 'Contractor Address' token.");
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

BaseAddress::Shared InitTrustLineCommand::contractorAddress() const
noexcept
{
    return mContractorAddress;
}