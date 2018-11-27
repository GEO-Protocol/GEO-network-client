#include "ShareKeysCommand.h"

ShareKeysCommand::ShareKeysCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "ShareKeysCommand: can't parse command. "
                "Received command is to short.");
    }

    try {
        string hexUUID = command.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "ShareKeysCommand: can't parse command. "
                "Error occurred while parsing 'Contractor UUID' token.");
    }

    // todo : parse contractorID

    size_t equivalentOffset = NodeUUID::kHexSize + 1;
    string equivalentStr = command.substr(
        equivalentOffset,
        command.size() - equivalentOffset - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
            "ShareKeysCommand: can't parse command. "
                "Error occurred while parsing  'equivalent' token.");
    }
}

const string &ShareKeysCommand::identifier()
    noexcept
{
    static const string identifier = "SET:contractors/trust-line-keys";
    return identifier;
}

const NodeUUID &ShareKeysCommand::contractorUUID() const
    noexcept
{
    return mContractorUUID;
}

const ContractorID ShareKeysCommand::contractorID() const
    noexcept
{
    return mContractorID;
}

const SerializedEquivalent ShareKeysCommand::equivalent() const
    noexcept
{
    return mEquivalent;
}