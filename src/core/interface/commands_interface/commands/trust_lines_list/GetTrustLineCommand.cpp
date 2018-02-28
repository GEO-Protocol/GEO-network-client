#include "GetTrustLineCommand.h"

GetTrustLineCommand::GetTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
    noexcept:
    BaseUserCommand(
        uuid,
        identifier())
{
    // This command does not requires any parameters. Command UUID and Identifier are parsed separately
    const auto minCommandLength = NodeUUID::kHexSize + 2;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "GetTrustLineCommand: can't parse command. "
                    "Received command is to short.");
    }

    try {
        string hexUUID = commandBuffer.substr(
            0,
            NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
                "GetTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor UUID' token.");
    }

    size_t equivalentOffset = NodeUUID::kHexSize + 1;
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

NodeUUID GetTrustLineCommand::contractorUUID()
{
    return mContractorUUID;
}

const SerializedEquivalent GetTrustLineCommand::equivalent() const
{
    return mEquivalent;
}
