#include "GetTrustLineCommand.h"

GetTrustLineCommand::GetTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
    noexcept:
    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 7;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "GetTrustLineCommand: can't parse command. "
                    "Received command is to short.");
    }

    auto tokenSeparatorPos = commandBuffer.find(kTokensSeparator);
    auto addressWithTypeStr = commandBuffer.substr(0, tokenSeparatorPos);
    auto addressTypeSeparatorPos = addressWithTypeStr.find(kAddressTypeSeparator);
    auto addressTypeStr = addressWithTypeStr.substr(0, addressTypeSeparatorPos);
    auto addressType = (BaseAddress::AddressType)std::stoul(addressTypeStr);
    auto addressStr = addressWithTypeStr.substr(
        addressTypeSeparatorPos + 1,
        addressWithTypeStr.length() - addressTypeSeparatorPos + 1);
    switch (addressType) {
        case BaseAddress::IPv4_IncludingPort: {
            mContractorAddress = make_shared<IPv4WithPortAddress>(
                addressStr);
            break;
        }
        default:
            throw ValueError(
                    "GetTrustLineCommand: can't parse command. "
                        "Error occurred while parsing 'Contractor Address' token.");
    }

    size_t equivalentOffset = tokenSeparatorPos + 1;
    string equivalentStr = commandBuffer.substr(
        equivalentOffset,
        commandBuffer.size() - equivalentOffset - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "GetTrustLineCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &GetTrustLineCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines/one";
    return kIdentifier;
}

CommandResult::SharedConst GetTrustLineCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        200,
        neighbor);
}

BaseAddress::Shared GetTrustLineCommand::contractorAddress() const
{
    return mContractorAddress;
}

const SerializedEquivalent GetTrustLineCommand::equivalent() const
{
    return mEquivalent;
}
