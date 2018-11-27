#include "SetOutgoingTrustLineCommand.h"


SetOutgoingTrustLineCommand::SetOutgoingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    static const auto minCommandLength = amountTokenOffset + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "SetTrustLineCommand: can't parse command. "
            "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "SetOutgoingTrustLineCommand: can't parse command. "
            "Error occurred while parsing 'Contractor UUID' token.");
    }

    // todo : parse contractorID

    size_t tokenSeparatorPos = command.find(
            kTokensSeparator,
            amountTokenOffset);
    try {
        mAmount = TrustLineAmount(
            command.substr(
                amountTokenOffset,
                tokenSeparatorPos - amountTokenOffset));

    } catch (...) {
        throw ValueError(
                "SetTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'New amount' token.");
    }

    size_t equivalentOffset = tokenSeparatorPos + 1;
    string equivalentStr = command.substr(
        equivalentOffset,
        command.size() - equivalentOffset - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "SetTrustLineCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &SetOutgoingTrustLineCommand::identifier()
    noexcept
{
    static const string identifier = "SET:contractors/trust-lines";
    return identifier;
}

const NodeUUID &SetOutgoingTrustLineCommand::contractorUUID() const
    noexcept
{
    return mContractorUUID;
}

const ContractorID SetOutgoingTrustLineCommand::contractorID() const
    noexcept
{
    return mContractorID;
}

const TrustLineAmount &SetOutgoingTrustLineCommand::amount() const
    noexcept
{
    return mAmount;
}

const SerializedEquivalent SetOutgoingTrustLineCommand::equivalent() const
    noexcept
{
    return mEquivalent;
}
