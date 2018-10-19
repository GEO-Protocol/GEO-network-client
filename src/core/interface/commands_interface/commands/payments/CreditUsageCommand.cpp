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

    size_t amountStartPos = NodeUUID::kHexSize+1;
    size_t tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        amountStartPos);
    try {
        mAmount = TrustLineAmount(
            commandBuffer.substr(
                amountStartPos,
                tokenSeparatorPos - amountStartPos));

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

    size_t equivalentStartPoint = tokenSeparatorPos + 1;
    string equivalentStr = commandBuffer.substr(
        equivalentStartPoint,
        commandBuffer.size() - equivalentStartPoint - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "CreditUsageCommand: can't parse command. "
                    "Error occurred while parsing  'equivalent' token.");
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

const SerializedEquivalent CreditUsageCommand::equivalent() const
{
    return mEquivalent;
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