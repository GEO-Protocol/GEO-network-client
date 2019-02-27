#include "CreditUsageCommand.h"


CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address,amount;
    uint32_t addressType, equivalentID, flag_amount = 0;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("CreditUsageCommand: there is no input ");
        }
    };
    auto parserType = [&](auto &ctx) {
        addressType = _attr(ctx);
    };
    auto address_add = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto address_number_add = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto address_Count = [&](auto &ctx) {
        mContractorAddressesCount = _attr(ctx);
    };
    auto addamount = [&](auto &ctx) {
        amount += _attr(ctx);
        flag_amount++;
        if (flag_amount > 39) { throw ValueError("Amount is too big"); }
        else if (flag_amount == 1 && _attr(ctx) <= 0) {
            throw ValueError("Amount can't be zero or low");
        }
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
                throw ValueError("CreditUsageCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");

        }
        address.erase();
    };

    auto equivalentID_add = [&](auto &ctx) {
        equivalentID = _attr(ctx);
    };

    try {
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            char_[check]);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(),
            *(int_[address_Count]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        mContractorAddresses.reserve(mContractorAddressesCount);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_[address_Count]) > char_(kTokensSeparator)
                > repeat(mContractorAddressesCount)[*(int_[parserType] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                > repeat(3)[int_[address_number_add]> char_('.') [address_add]]
                > int_[address_number_add] > char_(':') [address_add]
                > int_[address_number_add] > char_(kTokensSeparator) [address_vector]]
                >*(digit [addamount] > !alpha > !punct)
                > char_(kTokensSeparator)
                > +(int_[equivalentID_add]) > eol));
    } catch(...) {
        throw ValueError("CreditUsageCommand: can't parse command.");
    }
    mAmount = TrustLineAmount(amount);
    mEquivalent = equivalentID;
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