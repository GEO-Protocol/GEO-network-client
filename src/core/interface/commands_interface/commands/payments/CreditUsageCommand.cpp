#include "CreditUsageCommand.h"


CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    static const auto minCommandLength = 7;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "CreditUsageCommand::parse: "
                    "can't parse command. "
                    "Received command is too short.");
    }

    size_t contractorAddressStartPos = 0;
    size_t tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        contractorAddressStartPos);
    auto contractorAddressStr = commandBuffer.substr(
        contractorAddressStartPos,
        tokenSeparatorPos - contractorAddressStartPos);
    try {
        mContractorAddress = make_shared<IPv4WithPortAddress>(
            contractorAddressStr);
    } catch (...) {
        throw ValueError(
                "CreditUsageCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
    }

    size_t amountStartPos = tokenSeparatorPos + 1;
    tokenSeparatorPos = commandBuffer.find(
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

BaseAddress::Shared CreditUsageCommand::contractorAddress() const
{
    return mContractorAddress;
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