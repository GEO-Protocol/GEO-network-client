#include "CreditUsageCommand.h"

CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address, amount, addressType;
    size_t contractorAddressesCount;
    uint32_t flagAmount = 0;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator || _attr(ctx) == kTokensSeparator) {
            throw ValueError("CreditUsageCommand: input is empty.");
        }
    };
    auto addressTypeParse = [&](auto &ctx) {
        addressType += _attr(ctx);
    };
    auto addressAddChar = [&](auto &ctx) {
        address += _attr(ctx);
    };
    auto addressAddNumber = [&](auto &ctx) {
        address += std::to_string(_attr(ctx));
    };
    auto addressesCountParse = [&](auto &ctx) {
        contractorAddressesCount = _attr(ctx);
    };
    auto amountAddNumber = [&](auto &ctx) {
        amount += _attr(ctx);
        flagAmount++;
       if (flagAmount == 1 && _attr(ctx) == '0') {
            throw ValueError("CreditUsageCommand: amount contains leading zero.");
        }
    };
    auto addressAddToVector = [&](auto &ctx) {
        switch (std::atoi(addressType.c_str())) {
            case BaseAddress::IPv4_IncludingPort: {
                mContractorAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                        address));
                addressType.erase();
                break;
            }
            case BaseAddress::GNS: {
                mContractorAddresses.push_back(
                    make_shared<GNSAddress>(
                        address));
                addressType.erase();
                break;
            }
            default:
                throw ValueError("CreditUsageCommand: cannot parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
        }

        address.erase();
    };

    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };
    auto payloadParse = [&](auto &ctx) {
        mPayload += _attr(ctx);
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
        mContractorAddresses.reserve(contractorAddressesCount);
        parse(
            commandBuffer.begin(),
            commandBuffer.end(), (
                *(int_) > char_(kTokensSeparator)
                > addressLexeme<
                    decltype(addressAddChar),
                    decltype(addressAddNumber),
                    decltype(addressTypeParse),
                    decltype(addressAddToVector)>(
                        contractorAddressesCount,
                        addressAddChar,
                        addressAddNumber,
                        addressTypeParse,
                        addressAddToVector)
                >*(digit [amountAddNumber] > !alpha > !punct)
                > char_(kTokensSeparator)
                > +(int_[equivalentParse])
                > -(char_(kTokensSeparator) > *(char_[payloadParse] - eol))
                > eol > eoi));
        mAmount = TrustLineAmount(amount);
        if (mPayload.length() > std::numeric_limits<PayloadLength>::max()) {
            throw ValueError("Payload length is too big");
        }
    } catch(...) {
        throw ValueError("CreditUsageCommand: cannot parse command.");
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

const std::string CreditUsageCommand::payload() const
{
    return mPayload;
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