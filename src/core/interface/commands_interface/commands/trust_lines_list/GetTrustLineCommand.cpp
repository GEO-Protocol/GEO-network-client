#include "GetTrustLineCommand.h"

GetTrustLineCommand::GetTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer)
    noexcept:
    BaseUserCommand(
        uuid,
        identifier())
{
    parse(commandBuffer);
}

const string &GetTrustLineCommand::identifier()
{
    static const string kIdentifier = "GET:contractors/trust-lines/one";
    return kIdentifier;
}

CommandResult::SharedConst GetTrustLineCommand::resultOk(
    string &neighbor) const
{
    return make_shared<const CommandResult>(UUID(), 200, neighbor);
}

void GetTrustLineCommand::parse(
    const string &command)
{
    // This command does not requires any parameters. Command UUID and Identifier are parsed separately
    const auto minCommandLength = NodeUUID::kHexSize + 1;

    if (command.size() < minCommandLength) {
        throw ValueError("GetTrustLineCommand::parse: "
                             "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = command.substr(
            0,
            NodeUUID::kHexSize
        );
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError("GetTrustLineCommand::parse: "
                             "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }
}

NodeUUID GetTrustLineCommand::contractorUUID()
{
    return mContractorUUID;
}
