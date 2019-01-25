#include "InitiateMaxFlowCalculationCommand.h"

InitiateMaxFlowCalculationCommand::InitiateMaxFlowCalculationCommand(
    const CommandUUID &uuid,
    const string &command):

    BaseUserCommand(
        uuid,
        identifier())
{
    std::string address;
    uint32_t addressType, equivalentID;
    auto check = [&](auto &ctx) { if(_attr(ctx) == '\n'){throw ValueError("InitiateMaxFlowCalculationCommand: there is no input ");}};
    auto parserType = [&](auto &ctx) { addressType = _attr(ctx); };
    auto address_add = [&](auto &ctx) { address += _attr(ctx); };
    auto address_number_add = [&](auto &ctx) { address += std::to_string(_attr(ctx)); };
    auto address_Count = [&](auto &ctx) { mContractorsCount = _attr(ctx); };
    auto address_vector = [&](auto &ctx)
    {

        switch (addressType)
        {
            case BaseAddress::IPv4_IncludingPort:
            {
                mContractorAddresses.push_back(
                        make_shared<IPv4WithPortAddress>(
                                address));
                break;

            }
        }

        address.erase();
    };

    auto equivalentID_add = [&](auto &ctx) { equivalentID = _attr(ctx); };

    try
    {
        parse(command.begin(), command.end(), char_[check]);

        parse(command.begin(), command.end(), *(int_[address_Count]-char_('\t')) > char_('\t'));

        mContractorAddresses.reserve(mContractorsCount);

        parse(command.begin(), command.end(),
              (
                      *(int_[address_Count]) > char_('\t')
                      > repeat(mContractorsCount)[*(int_[parserType] - char_('\t')) > char_('\t')
                                               > repeat(3)[int_[address_number_add]> char_('.') [address_add]]
                                               > int_[address_number_add] > char_(':') [address_add]
                                               > int_[address_number_add] > char_('\t') [address_vector]]
                      > +(int_[equivalentID_add]) > eol
              )
        );

    }
    catch(...)
    {
        throw ValueError("InitTrustLineCommand: can't parse command.");
    }
    mEquivalent = equivalentID;
}

const string &InitiateMaxFlowCalculationCommand::identifier()
{
    static const string identifier = "GET:contractors/transactions/max";
    return identifier;
}

const vector<BaseAddress::Shared>& InitiateMaxFlowCalculationCommand::contractorAddresses() const
{
    return mContractorAddresses;
}

const SerializedEquivalent InitiateMaxFlowCalculationCommand::equivalent() const
{
    return mEquivalent;
}

CommandResult::SharedConst InitiateMaxFlowCalculationCommand::responseOk(
    string &maxFlowAmount) const
{
    return CommandResult::SharedConst(
        new CommandResult(
            identifier(),
            UUID(),
            200,
            maxFlowAmount));
}