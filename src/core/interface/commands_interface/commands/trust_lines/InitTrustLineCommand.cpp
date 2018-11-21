#include "InitTrustLineCommand.h"

InitTrustLineCommand::InitTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "InitTrustLineCommand: can't parse command. "
                "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "InitTrustLineCommand: can't parse command. "
                "Error occurred while parsing 'Contractor UUID' token.");
    }

    size_t tokenSeparatorPos = NodeUUID::kHexSize;
    size_t nextTokenSeparatorPos = command.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    mContractorAddress = command.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);

    size_t equivalentOffset = nextTokenSeparatorPos + 1;
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

const NodeUUID &InitTrustLineCommand::contractorUUID() const
noexcept
{
    return mContractorUUID;
}

const SerializedEquivalent InitTrustLineCommand::equivalent() const
noexcept
{
    return mEquivalent;
}

const string InitTrustLineCommand::contractorAddress() const
noexcept
{
    return mContractorAddress;
}