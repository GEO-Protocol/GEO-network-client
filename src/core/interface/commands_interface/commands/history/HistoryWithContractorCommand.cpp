#include "HistoryWithContractorCommand.h"

HistoryWithContractorCommand::HistoryWithContractorCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    const auto minCommandLength = 7;
    if (commandBuffer.size() < minCommandLength) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Received command is to short.");
    }

    size_t tokenSeparatorPos = commandBuffer.find(
        kTokensSeparator);
    string historyFromStr = commandBuffer.substr(
        0,
        tokenSeparatorPos);
    try {
        mHistoryFrom = std::stoul(historyFromStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing  'from' token.");
    }

    size_t nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    string historyCountStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mHistoryCount = std::stoul(historyCountStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing 'count' token.");
    }

    nextTokenSeparatorPos = commandBuffer.find(
        kTokensSeparator,
        tokenSeparatorPos + 1);
    auto contractorAddressesCntStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mContractorsCount = (uint32_t)std::stoul(contractorAddressesCntStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing  'contractor addresses count' token.");
    }

    mContractorAddresses.reserve(mContractorsCount);
    size_t contractorAddressStartPos = nextTokenSeparatorPos + 1;
    for (size_t idx = 0; idx < mContractorsCount; idx++) {
        try {
            tokenSeparatorPos = commandBuffer.find(
                kTokensSeparator,
                contractorAddressStartPos);
            string addressTypeStr = commandBuffer.substr(
                contractorAddressStartPos,
                tokenSeparatorPos - contractorAddressStartPos);
            auto addressType = (BaseAddress::AddressType)std::stoul(addressTypeStr);

            contractorAddressStartPos = tokenSeparatorPos + 1;
            tokenSeparatorPos = commandBuffer.find(
                kTokensSeparator,
                contractorAddressStartPos);
            string addressStr = commandBuffer.substr(
                contractorAddressStartPos,
                tokenSeparatorPos - contractorAddressStartPos);

            switch (addressType) {
                case BaseAddress::IPv4_IncludingPort: {
                    mContractorAddresses.push_back(
                        make_shared<IPv4WithPortAddress>(
                            addressStr));
                    break;
                }
                default:
                    throw ValueError(
                            "HistoryWithContractorCommand: can't parse command. "
                                "Error occurred while parsing 'Contractor Address' token.");
            }
            contractorAddressStartPos = tokenSeparatorPos + 1;
        } catch (...) {
            throw ValueError(
                    "HistoryWithContractorCommand: can't parse command. "
                        "Error occurred while parsing 'Contractor Address' token.");
        }
    }

    nextTokenSeparatorPos = commandBuffer.size() - 1;
    string equivalentStr = commandBuffer.substr(
        tokenSeparatorPos + 1,
        nextTokenSeparatorPos - tokenSeparatorPos - 1);
    try {
        mEquivalent = (uint32_t)std::stoul(equivalentStr);
    } catch (...) {
        throw ValueError(
                "HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing 'equivalent' token.");
    }
}

const string &HistoryWithContractorCommand::identifier()
{
    static const string identifier = "GET:history/contractor";
    return identifier;
}

const size_t HistoryWithContractorCommand::historyFrom() const
{
    return mHistoryFrom;
}

const size_t HistoryWithContractorCommand::historyCount() const
{
    return mHistoryCount;
}

const vector<BaseAddress::Shared>& HistoryWithContractorCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const SerializedEquivalent HistoryWithContractorCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst HistoryWithContractorCommand::resultOk(
    string &historyPaymentsStr) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            historyPaymentsStr));
}
