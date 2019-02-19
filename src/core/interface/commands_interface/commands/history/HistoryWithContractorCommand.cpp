#include "HistoryWithContractorCommand.h"

HistoryWithContractorCommand::HistoryWithContractorCommand(
    const CommandUUID &uuid,
    const string &commandBuffer):

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address;
    uint32_t addressType;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("HistoryWithContractorCommand: there is no input ");
        }
    };
    auto historyFrom_add = [&](auto &ctx) {
        mHistoryFrom = _attr(ctx);
    };
    auto historyCount_add = [&](auto &ctx) {
        mHistoryCount = _attr(ctx);
    };
    auto contractorsCount_add = [&](auto &ctx) {
        mContractorsCount = _attr(ctx);
    };
    auto parserType = [&](auto &ctx) {
        addressType = _attr(ctx);
    };
    auto equivalentID_add = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };
    auto address_add = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto address_number_add = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto address_vector = [&](auto &ctx) {
        switch (addressType) {
            case BaseAddress::IPv4_IncludingPort: {
                mContractorAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                        address));
                break;
            }
            default:
                throw ValueError("HistoryWithContractorCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[historyFrom_add] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                > *(int_[historyCount_add] - char_(kTokensSeparator))
                > char_(kTokensSeparator)  > *(int_[contractorsCount_add] - char_(kTokensSeparator))));
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_) > char_(kTokensSeparator)  >*(int_)
                > char_(kTokensSeparator)  > *(int_)
                > char_(kTokensSeparator)
                > repeat(mContractorsCount)[*(int_[parserType] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                >repeat(3)[int_[address_number_add]>char_('.') [address_add]]
                >int_[address_number_add]
                >char_(':') [address_add]
                > int_[address_number_add]
                > char_(kTokensSeparator)  [address_vector]]
                > +(int_[equivalentID_add]) > eol));
    } catch (...) {
        throw ValueError("HistoryWithContractorCommand: can't parse command.");
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
