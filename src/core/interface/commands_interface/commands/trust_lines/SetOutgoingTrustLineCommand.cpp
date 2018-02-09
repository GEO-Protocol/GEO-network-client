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

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < command.length(); ++commandSeparatorPosition) {
            if (command.at(commandSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(
                    command.substr(
                        amountTokenOffset,
                        commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError(
            "SetTrustLineCommand: can't parse command. "
            "Error occurred while parsing 'New amount' token.");
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

const TrustLineAmount &SetOutgoingTrustLineCommand::amount() const
    noexcept
{
    return mAmount;
}
