#include "InitiateMaxFlowCalculationFullyCommand.h"

InitiateMaxFlowCalculationFullyCommand::InitiateMaxFlowCalculationFullyCommand(
    const CommandUUID &uuid,
    const string &command):

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address;
    uint32_t addressType;
    auto check = [&](auto &ctx) {
        if(_attr(ctx) == kCommandsSeparator) {
            throw ValueError("InitiateMaxFlowCalculationFullyCommand: there is no input ");
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
        mContractorsCount = _attr(ctx);
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
                throw ValueError("InitiateMaxFlowCalculationFullyCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
        }
        address.erase();
    };
    auto equivalentParse = [&](auto &ctx) {
        mEquivalent = _attr(ctx);
    };

    try {
        parse(
            command.begin(),
            command.end(),
            char_[check]);
        parse(
            command.begin(),
            command.end(),
            *(int_[addressesCountParse]-char_(kTokensSeparator)) > char_(kTokensSeparator));
        mContractorAddresses.reserve(mContractorsCount);
        parse(
            command.begin(),
            command.end(), (
                *(int_[addressesCountParse]) > char_(kTokensSeparator)
                > repeat(mContractorsCount)[*(int_[addressTypeParse] - char_(kTokensSeparator)) > char_(kTokensSeparator)
                > repeat(3)[int_[addressAddNumber]> char_('.') [addressAddChar]]
                > int_[addressAddNumber] > char_(':') [addressAddChar]
                > int_[addressAddNumber] > char_(kTokensSeparator) [addressAddToVector]]
                > +(int_[equivalentParse]) > eol));
    } catch(...) {
        throw ValueError("InitTrustLineCommand: can't parse command.");
    }
}

const string &InitiateMaxFlowCalculationFullyCommand::identifier()
{
    static const string identifier = "GET:contractors/transactions/max/fully";
    return identifier;
}

const vector<BaseAddress::Shared>& InitiateMaxFlowCalculationFullyCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const SerializedEquivalent InitiateMaxFlowCalculationFullyCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst InitiateMaxFlowCalculationFullyCommand::responseOk(
    string &maxFlowAmount) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            maxFlowAmount));
}
