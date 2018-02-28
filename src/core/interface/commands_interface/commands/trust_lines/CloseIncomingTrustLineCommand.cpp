#include "CloseIncomingTrustLineCommand.h"

CloseIncomingTrustLineCommand::CloseIncomingTrustLineCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "CloseIncomingTrustLineCommand: can't parse command. "
                    "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "CloseIncomingTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor UUID' token.");
    }

    size_t equivalentOffset = NodeUUID::kHexSize + 1;
    string equivalentStr = command.substr(
        equivalentOffset,
        command.size() - equivalentOffset - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "CloseIncomingTrustLineCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
    }
}

const string &CloseIncomingTrustLineCommand::identifier()
    noexcept
{
    static const string identifier = "DELETE:contractors/incoming-trust-line";
    return identifier;
}

const NodeUUID &CloseIncomingTrustLineCommand::contractorUUID() const
    noexcept
{
    return mContractorUUID;
}

const SerializedEquivalent CloseIncomingTrustLineCommand::equivalent() const
    noexcept
{
    return mEquivalent;
}
