#include "CreditUsageCommand.h"


CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address, amount;
    uint32_t addressType, flagAmount = 0;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("CreditUsageCommand: there is no input ");
        }
    };
    auto addressTypeParse = [&](auto &ctx) {
        addressType = _attr(ctx);
    };
    auto addressAddChar = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto addressAddNumber = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto addressesCountParse = [&](auto &ctx) {
        mContractorAddressesCount = _attr(ctx);
    };
    auto amountAddNumber = [&](auto &ctx) {
        amount += _attr(ctx);
        flagAmount++;
        if (flagAmount > 39) { throw ValueError("Amount is too big"); }
        else if (flagAmount == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
        }
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
                throw ValueError("CreditUsageCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");

        }
        address.erase();
    };

    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            *(int_[addressesCountParse]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        mContractorAddresses.reserve(mContractorAddressesCount);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[addressesCountParse]) > char_(kTokensSeparator)
                > repeat(mContractorAddressesCount)[*(int_[addressTypeParse] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                > repeat(3)[int_[addressAddNumber]> char_('.') [addressAddChar]]
                > int_[addressAddNumber] > char_(':') [addressAddChar]
                > int_[addressAddNumber] > char_(kTokensSeparator) [addressAddToVector]]
                >*(digit [amountAddNumber] > !alpha > !punct)
                > char_(kTokensSeparator)
                > +(int_[equivalentParse]) > eol));
        mAmount = TrustLineAmount(amount);
    } catch(...) {
        throw ValueError("CreditUsageCommand: can't parse command.");
    }
}

const string& CreditUsageCommand::identifier()
{
    static const string identifier = "CREATE:contractors/transactions";
    return identifier;
}

vector<BaseAddress::Shared> CreditUsageCommand::contractorAddresses() const
{
    return mContractorAddresses;
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