#include "OpenTrustLineCommand.h"


OpenTrustLineCommand::OpenTrustLineCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto amountTokenOffset = NodeUUID::kHexSize + 1;
    const auto minCommandLength = amountTokenOffset + 1;

    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
            "OpenTrustLineCommand::parse: "
            "Can't parse command. Received command is to short.");
    }

    try {
        string hexUUID = commandBuffer.substr(0, NodeUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "OpenTrustLineCommand::parse: "
            "can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    try {
        for (size_t commandSeparatorPosition = amountTokenOffset; commandSeparatorPosition < commandBuffer.length(); ++commandSeparatorPosition) {
            if (commandBuffer.at(commandSeparatorPosition) == kCommandsSeparator) {
                mAmount = TrustLineAmount(
                    commandBuffer.substr(
                        amountTokenOffset,
                        commandSeparatorPosition - amountTokenOffset));
            }
        }

    } catch (...) {
        throw ValueError(
            "OpenTrustLineCommand::parse: "
            "can't parse command. Error occurred while parsing 'Amount' token.");
    }

    if (mAmount == TrustLineAmount(0)){
        throw ValueError(
            "OpenTrustLineCommand::parse: "
            "Can't parse command. Received 'Amount' can't be 0.");
    }
}

const string &OpenTrustLineCommand::identifier()
{
    static const string identifier = "CREATE:contractors/trust-lines";
    return identifier;
}

const NodeUUID &OpenTrustLineCommand::contractorUUID() const
{
    return mContractorUUID;
}

const TrustLineAmount &OpenTrustLineCommand::amount() const
{
    return mAmount;
}
