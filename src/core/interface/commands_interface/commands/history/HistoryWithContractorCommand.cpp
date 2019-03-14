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
    auto historyFromParse = [&](auto &ctx) {
        mHistoryFrom = _attr(ctx);
    };
    auto historyCountParse = [&](auto &ctx) {
        mHistoryCount = _attr(ctx);
    };
    auto contractorAddressesCountParse = [&](auto &ctx) {
        mContractorAddressesCount = _attr(ctx);
    };
    auto addressTypeParse = [&](auto &ctx) {
        addressType = _attr(ctx);
    };
    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };
    auto addressAddChar = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto addressAddNumber = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto addressAddToVector = [&](auto &ctx) {
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
                *(int_[historyFromParse] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                > *(int_[historyCountParse] - char_(kTokensSeparator))
                > char_(kTokensSeparator)  > *(int_[contractorAddressesCountParse] - char_(kTokensSeparator))));
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_) > char_(kTokensSeparator)  >*(int_)
                > char_(kTokensSeparator)  > *(int_)
                > char_(kTokensSeparator)
                > repeat(mContractorAddressesCount)[*(int_[addressTypeParse] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                >repeat(3)[int_[addressAddNumber]>char_('.') [addressAddChar]]
                >int_[addressAddNumber]
                >char_(':') [addressAddChar]
                > int_[addressAddNumber]
                > char_(kTokensSeparator)  [addressAddToVector]]
                > +(int_[equivalentParse]) > eol));
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
