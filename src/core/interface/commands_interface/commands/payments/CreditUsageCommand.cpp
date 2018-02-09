#include "CreditUsageCommand.h"


CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    static const auto minCommandLength = CommandUUID::kHexSize + 2;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "CreditUsageCommand::parse: "
                    "can't parse command. "
                    "Received command is too short.");
    }

    try {
        string hexUUID = commandBuffer.substr(
            0,
            CommandUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
                "CreditUsageCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor UUID' token.");
    }

    for (size_t i = NodeUUID::kHexSize+1; i < commandBuffer.length(); ++i) {
        if (commandBuffer.at(i) == kTokensSeparator ||
            commandBuffer.at(i) == kCommandsSeparator ||
            i == commandBuffer.length()-1) {

            try {
                mAmount = TrustLineAmount(
                    commandBuffer.substr(
                        NodeUUID::kHexSize+1,
                        i - NodeUUID::kHexSize-1));

            } catch (...) {
                throw ValueError(
                        "CreditUsageCommand: can't parse command. "
                            "Error occurred while parsing 'Amount' token.");
            }

            if (mAmount == TrustLineAmount(0)) {
                throw ValueError(
                        "CreditUsageCommand: can't parse command. "
                            "Received 'Amount' can't be 0.");
            }

            // Command amount parsed well
            break;
        }
    }
}

const string& CreditUsageCommand::identifier()
{
    static const string identifier = "CREATE:contractors/transactions";
    return identifier;
}

const NodeUUID& CreditUsageCommand::contractorUUID() const
{
    return mContractorUUID;
}

const TrustLineAmount& CreditUsageCommand::amount() const
{
    return mAmount;
}

CommandResult::SharedConst CreditUsageCommand::responseNoConsensus () const
{
    return makeResult(409);
}

CommandResult::SharedConst CreditUsageCommand::responseOK(
    string &transactionUUID) const
{
    return make_shared<const CommandResult>(
        identifier(),
        UUID(),
        201,
        transactionUUID);
}