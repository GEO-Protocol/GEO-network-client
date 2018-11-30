#include "ShareKeysCommand.h"

ShareKeysCommand::ShareKeysCommand(
    const CommandUUID &commandUUID,
    const string &command) :

    BaseUserCommand(
        commandUUID,
        identifier())
{
    static const auto minCommandLength = 3;

    if (command.size() < minCommandLength) {
        throw ValueError(
            "ShareKeysCommand: can't parse command. "
                "Received command is to short.");
    }

    size_t tokenSeparatorPos = command.find(
        kTokensSeparator,
        0);
    string contractorIDStr = command.substr(0, tokenSeparatorPos);
    try {
        mContractorID = (uint32_t)std::stoul(contractorIDStr);
    } catch (...) {
        throw ValueError(
            "ShareKeysCommand: can't parse command. "
                "Error occurred while parsing 'contractorID' token.");
    }

    size_t equivalentOffset = tokenSeparatorPos + 1;
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